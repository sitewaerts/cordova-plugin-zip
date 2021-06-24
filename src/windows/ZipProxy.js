var JSZip = require("./JsZip");

var Storage = Windows.Storage; // Alias for readability


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
 * @param {string} filename
 * @param {string} outputDir
 * @param {function(total:number, loaded:number):void} progressCallback
 * @return {PromiseLike<any>}
 */
function unzipJSZip(filename, outputDir, progressCallback)
{
    var fileCollisionOption = Storage.CreationCollisionOption.replaceExisting;

    return Storage.StorageFile
        .getFileFromApplicationUriAsync(new Windows.Foundation.Uri(filename))
        .then(getFileAsUint8Array)
        .then(function (zipFileContents)
        {
            return resolveOutDir(outputDir)
                .then(function (outFolder)
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
                                    return outFolder.createFileAsync(zippedFile.name.replace(/\//g, '\\'), fileCollisionOption)
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
        });
}

/**
 * @param {string} filename
 * @param {string} outputDir
 * @param {string} zipAlgorithm ('miniz-cpp' or 'andyzip')
 * @param {function(total:number, loaded:number):void} progressCallback
 * @return {PromiseLike<any>}
 */
function unzipUWP(zipFileIn, outDirIn, zipAlgorithm, progressCallback)
{
    console.log('ZipProxy.Zip.unzip.unzipUWP()');
    console.log('- zipFileIn = ' + zipFileIn);
    console.log('- outDirIn = ' + outDirIn);
    console.log('- zipAlgorithm = ' + zipAlgorithm);

    return Storage.StorageFile
        .getFileFromApplicationUriAsync(new Windows.Foundation.Uri(zipFileIn))
        .then(function(zipFileNative)
        {
            console.log('- zipFileNative.path = ' + zipFileNative.path);

            return resolveOutDir(outDirIn)
				.then(function(outDirNative)
				{
                    console.log('- outDirNative.path = ' + outDirNative.path);

					try
                    {
                        var zipUWP = new ZipComponentUWP.ZipComponent(zipAlgorithm, zipFileNative.path, outDirNative.path);
						var zipFileIndex = 0;
						var zipFileCount = zipUWP.getEntryCount();

						progressCallback(zipFileCount, 0);

						function _fileProcessed()
						{
							progressCallback(zipFileCount, ++zipFileIndex);
						}

						var zipJobs = [];
						for(let i = 0; i < zipFileCount; ++i)
							zipJobs[i] = i;

						return WinJS.Promise.join(zipJobs.map(function(zipJob)
							{
								return new WinJS.Promise(function(resolve)
									{
										zipUWP.unzipEntry(zipJob);
										resolve();
									}).then(_fileProcessed);
							}
						)).then(function ()
						{
							progressCallback(zipJobs.length, zipJobs.length);
						});
					}
					catch(error)
					{
						console.error("cannot unzip", error);
						return WinJS.Promise.wrapError(error);
					}
				});
        });
}

cordova.commandProxy.add("Zip", {
    unzip: function (successCallback, errorCallback, args)
    {
        if (!args || !args.length)
        {
            var message = "Error, something was wrong with the input filename. => " + filename;
            if(errorCallback)
                errorCallback(message);
            else
                console.error(message);
        }
		else
        {
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
			
			// Use the fastest algorithm 'andyzip' as default, but fallback to 'jszip'
			// if the current platform doesn't support the native UWP implementations
			let algorithm = args[2] || 'andyzip';
			if( algorithm !== 'jszip' && !isWindows10orHigher() )
				algorithm = 'jszip';

            if( algorithm === 'jszip' ) {
				unzipJSZip(args[0], args[1], progressCallback)
                    .then(successCallback, errorCallback || console.error.bind(console));
            }
            else{
                unzipUWP(args[0], args[1], algorithm, progressCallback)
                    .then(successCallback, errorCallback || console.error.bind(console));
            }
        }
    }
});
