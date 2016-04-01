var JSZip = require("jszip");

function getFileAsUint8Array(file) {
    return storage.FileIO.readBufferAsync(file)
        .then(function (buffer) {
            //Read the file into a byte array
            var fileContents = new Uint8Array(buffer.length);
            var dataReader = storage.Streams.DataReader.fromBuffer(buffer);
            dataReader.readBytes(fileContents);
            dataReader.close();

            return fileContents;
        });
}

module.exports = {
    unzip : function(filename, outputDir){
        var fileCollisionOption = replaceIfExists ?
            storage.CreationCollisionOption.replaceExisting :
            storage.CreationCollisionOption.failIfExists;

        return storage.StorageFile
            .getFileFromPathAsync(filename)
            .then(getFileAsUint8Array)
            .then(function (zipFileContents) {
                //Create the zip data in memory
                var zip = new JSZip(zipFileContents);

                //Extract files
                var promises = [];
                var lf = storage.ApplicationData.current.localFolder;
                _.each(zip.files, function (zippedFile) {

                    //Create new file
                    promises.push(lf
                        .createFileAsync(zippedFile.name, fileCollisionOption)
                        .then(function (localStorageFile) {
                            //Copy the zipped file's contents into the local storage file
                            var fileContents = zip.file(zippedFile.name).asUint8Array();
                            return storage.FileIO
                                .writeBytesAsync(localStorageFile, fileContents);
                        })
                    );
                });

                return WinJS.Promise.join(promises);
            });
    }
};

require("cordova/exec/proxy").add("Zip",module.exports);