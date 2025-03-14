// noinspection ES6ConvertVarToLetConst

var JSZip = require("./JsZip"); // provided by plugin.xml

var Storage = Windows.Storage; // Alias for readability


/**
 *
 * @param {StorageFile} file
 * @return {Windows.Foundation.IPromise<Uint8Array>}
 */
function getFileAsUint8Array(file)
{
    return Storage.FileIO.readBufferAsync(file)
        .then(function (buffer)
        {
            //Read the file into a byte array
            var fileContents = new Uint8Array(buffer.length);
            var dataReader = Storage.Streams.DataReader.fromBuffer(buffer);
            dataReader.readBytes(fileContents);
            dataReader.close();

            return fileContents;
        });
}

/**
 * @param {string} zipFile
 * @return {WinJS.Promise<StorageFile>}
 */
function resolveZipFile(zipFile)
{
    return new WinJS.Promise(function (resolve, reject)
    {
        function _error(error)
        {
            error = {
                message: "cannot resolve",
                dir: zipFile,
                cause: error
            };
            console.error(error);
            reject(error);
        }

        function _success(result)
        {
            console.log("successfully resolved " + zipFile, result);
            resolve(result);
        }

        window.resolveLocalFileSystemURL(zipFile,
            function (file)
            {
                if(!file.isFile)
                    _error(zipFile + " is not a file");
                else
                    Storage.StorageFile
                        .getFileFromApplicationUriAsync(new Windows.Foundation.Uri(file.nativeURL))
                        .then(_success, _error);
            },
            _error
        );
    });
}

/**
 *
 * @param {string} outputDir
 * @return {WinJS.Promise<StorageFolder>}
 */
function resolveOutDir(outputDir)
{
    return new WinJS.Promise(function (resolve, reject)
    {
        function _error(error)
        {
            error = {
                message: "cannot resolve",
                dir: outputDir,
                cause: error
            };
            console.error(error);
            reject(error);
        }

        function _success(result)
        {
            console.log("successfully resolved " + outputDir, result);
            resolve(result);
        }

        window.resolveLocalFileSystemURL(outputDir,
            function (dir)
            {
                console.log(dir);

                var targetFolder;

                // cordova.file.cacheDirectory and cordova.file.tempDirectory overlap on windows ....
                // if (dir.nativeURL.indexOf(cordova.file.cacheDirectory) === 0)
                //     targetFolder = storage.ApplicationData.current.localCacheFolder;
                if (dir.nativeURL.indexOf(cordova.file.tempDirectory) === 0)
                    targetFolder = Storage.ApplicationData.current.temporaryFolder;
                else if (dir.nativeURL.indexOf(cordova.file.syncedDataDirectory) === 0)
                    targetFolder = Storage.ApplicationData.current.roamingFolder;
                else if (dir.nativeURL.indexOf(cordova.file.dataDirectory) === 0)
                    targetFolder = Storage.ApplicationData.current.localFolder;
                else
                {
                    _error("cannot map native url to file system: " + dir.nativeURL);
                    return;
                }

                targetFolder.getFolderAsync(dir.fullPath.substring(1).replace(/\//g, '\\')).then(_success, _error);
				//targetFolder.getFolderAsync(dir.fullPath).then(_success, _error);
            },
            _error
        );
    });
}

//TODO: check if zip-js is faster and uses less memory for large zips

/**
 * @param {StorageFile} inputFile
 * @param {StorageFolder} outputDir
 * @param {function(total:number, loaded:number):void} progressCallback
 * @return {PromiseLike<any>}
 */
function unzipJSZip(inputFile, outputDir, progressCallback)
{
    console.log('ZipProxy.Zip.unzip.unzipJSZip()');
    console.log('- filename = ' + inputFile.path);
    console.log('- outputDir = ' + outputDir.path);

    var fileCollisionOption = Storage.CreationCollisionOption.replaceExisting;

    return getFileAsUint8Array(inputFile)
        .then(function (zipFileContents)
        {
            try
            {
                //Create the zip data in memory
                return new JSZip().loadAsync(zipFileContents).then(function(zip){
                    var files = Object.values(zip.files);
                    var done = 0;
                    progressCallback(files.length, done);

                    function _fileProcessed()
                    {
                        progressCallback(files.length, ++done);
                    }

                    //Extract files
                    return WinJS.Promise.join(files.map(function (zippedFile)
                        {
                            if (zippedFile.dir)
                                return new WinJS.Promise(function (resolve)
                                {
                                    resolve()
                                }).then(_fileProcessed);
                            //Create new file
                            return outputDir.createFileAsync(zippedFile.name.replace(/\//g, '\\'), fileCollisionOption)
                                .then(function (localStorageFile)
                                {
                                    //Copy the zipped file's contents into the local storage file
                                    return zip.file(zippedFile.name).async("array")
                                        .then(function(fileContents){
                                            return Storage.FileIO.writeBytesAsync(localStorageFile, fileContents);
                                        });
                                }).then(_fileProcessed);
                        }
                    )).then(function ()
                    {
                        progressCallback(files.length, files.length);
                    });
                });
            }
            catch(error){
                console.error("cannot unzip", error);
                return WinJS.Promise.wrapError(error);
            }
        });
}

/**
 * @param {StorageFile} inputFile
 * @param {StorageFolder} outputDir
 * @param {string} zipAlgorithm ('miniz-cpp' or 'andyzip')
 * @param {function(total:number, loaded:number):void} progressCallback
 * @return {PromiseLike<any>}
 */
function unzipUWP(inputFile, outputDir, zipAlgorithm, progressCallback)
{
    console.log('ZipProxy.Zip.unzip.unzipUWP()');
    console.log('- inputFile = ' + inputFile.path);
    console.log('- outputDir = ' + outputDir.path);
    console.log('- zipAlgorithm = ' + zipAlgorithm);

    try
    {
        let zipUWP = new ZipComponentUWP.ZipComponent(zipAlgorithm, inputFile.path, outputDir.path);
        if( zipUWP == null ) {
            let initError = 'ERROR: Failed to initialize ZipComponentUWP!';
            console.log(initError);
            return WinJS.Promise.wrapError(initError);
        }

        console.log('- ZipComponentUWP Version = ' + zipUWP.getVersion());

        let initError = zipUWP.getLastError();
        if( initError !== '' ) {
            console.log(initError);
            return WinJS.Promise.wrapError(initError);
        }

        let zipFileCount = zipUWP.getEntryCount();
        if( zipFileCount === 0 ) {
            console.log('ERROR: No zip entries found! ' + zipUWP.getLastError());
            return WinJS.Promise.wrapError(zipUWP.getLastError());
        }

        progressCallback(zipFileCount, 0);

        let zipFileIndex = 0;
        function _fileProcessed() {
            progressCallback(zipFileCount, ++zipFileIndex);
        }

        let zipJobs = [];
        for(let i = 0; i < zipFileCount; ++i)
            zipJobs[i] = i;

        return WinJS.Promise.join(zipJobs.map(function(zipJob)
            {
                return new WinJS.Promise(function (completeDispatch, errorDispatch)
                {
                    if (zipUWP.unzipEntry(zipJob) === true) {
                        completeDispatch();
                    } else {
                        let errorName = 'ERROR: Failed to unzip entry [' + zipJob + '] = ' + zipUWP.getEntryName(zipJob);
                        let errorMessage = 'zipUWP.getLastError() = ' + zipUWP.getLastError();
                        console.log(errorName +' ' + errorMessage);
                        errorDispatch(new WinJS.ErrorFromName(errorName, errorMessage));
                    }
                }).then(_fileProcessed);
            }
        ))
            .then(function()
            {
                progressCallback(zipJobs.length, zipJobs.length);
            });
    }
    catch(error)
    {
        console.error("cannot unzip", error);
        return WinJS.Promise.wrapError(error);
    }

}

cordova.commandProxy.add("Zip", {
    unzip: function (successCallback, errorCallback, args)
    {
        function onError(e){
            if(errorCallback)
                errorCallback(e);
            else
                console.error(e);
        }

        if (!args || args.length < 2)
        {
            onError("Error, something was wrong with the arguments");
        }
		else
        {
            resolveZipFile(args[0]).then(
                function(inputFile){
                    resolveOutDir(args[1]).then(
                        function(outputDirectory){

                            function progressCallback(total, loaded){
                                successCallback({loaded: loaded, total: total}, { keepCallback: true});
                            }

                            function isWindows10orHigher() {
                                if( device && device.platform && device.platform === 'windows' && device.version ) {
                                    let windowsVersion = parseInt(device.version.substr(0, device.version.indexOf('.')));
                                    return windowsVersion >= 10;
                                }
                                return false;
                            }

                            // Use the algorithm 'miniz-cpp' as default, but fallback to 'jszip'
                            // if the current platform doesn't support the native UWP implementations
                            let algorithm = args[2] || 'miniz-cpp';
                            if( algorithm !== 'jszip' && !isWindows10orHigher() )
                                algorithm = 'jszip';

                            if( algorithm === 'jszip' ) {
                                unzipJSZip(inputFile, outputDirectory, progressCallback)
                                    .then(successCallback, onError);
                            }
                            else {
                                unzipUWP(inputFile, outputDirectory, algorithm, progressCallback)
                                    .then(successCallback, onError);
                            }
                        },
                        onError
                    );
                },
                onError
            );
        }
    }
});
