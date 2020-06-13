var JSZip = require("./JsZip");

var storage = Windows.Storage; // Alias for readability


function getFileAsUint8Array(file)
{
    return storage.FileIO.readBufferAsync(file)
        .then(function (buffer)
        {
            //Read the file into a byte array
            var fileContents = new Uint8Array(buffer.length);
            var dataReader = storage.Streams.DataReader.fromBuffer(buffer);
            dataReader.readBytes(fileContents);
            dataReader.close();

            return fileContents;
        });
}

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
                    targetFolder = storage.ApplicationData.current.temporaryFolder;
                else if (dir.nativeURL.indexOf(cordova.file.syncedDataDirectory) === 0)
                    targetFolder = storage.ApplicationData.current.roamingFolder;
                else if (dir.nativeURL.indexOf(cordova.file.dataDirectory) === 0)
                    targetFolder = storage.ApplicationData.current.localFolder;
                else
                {
                    _error("cannot map native url to file system: " + dir.nativeURL);
                    return;
                }

                targetFolder.getFolderAsync(dir.fullPath.substring(1).replace(/\//g, '\\')).then(_success, _error);
            },
            _error
        );

    });


}

function unzip(filename, outputDir)
{
    var fileCollisionOption = storage.CreationCollisionOption.replaceExisting;

    return storage.StorageFile
        .getFileFromApplicationUriAsync(new Windows.Foundation.Uri(filename))
        .then(getFileAsUint8Array)
        .then(function (zipFileContents)
        {
            return resolveOutDir(outputDir)
                .then(function (outFolder)
                {
                    //Create the zip data in memory
                    var zip = new JSZip(zipFileContents);

                    //Extract files
                    return WinJS.Promise.join(Object.values(zip.files).map(function (zippedFile)
                    {
                        if(zippedFile.dir)
                            return new WinJS.Promise(function(resolve){resolve()});
                        //Create new file
                        return outFolder.createFileAsync(zippedFile.name.replace(/\//g, '\\'), fileCollisionOption)
                            .then(function (localStorageFile)
                            {
                                //Copy the zipped file's contents into the local storage file
                                var fileContents = zip.file(zippedFile.name).asUint8Array();
                                return storage.FileIO
                                    .writeBytesAsync(localStorageFile, fileContents);
                            });
                    }));
                });
        });
}

cordova.commandProxy.add("Zip", {
    unzip: function (successCallback, errorCallback, args)
    {
        if (!args || !args.length)
        {
            var message = "Error, something was wrong with the input filename. =>" + filename;
            if(errorCallback)
                errorCallback(message);
            else
                console.error(message);
        }
        else
        {
            unzip(args[0], args[1]).then(successCallback, errorCallback || console.error.bind(console));
        }
    }
});
