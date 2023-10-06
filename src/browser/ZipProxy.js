// noinspection ES6ConvertVarToLetConst

var JSZip = require("./JsZip"); // provided by plugin.xml

/**
 * @param {string} filename
 * @param {string} outputDir
 * @param {function(total:number, loaded:number):void} progressCallback
 * @return {PromiseLike<any>}
 */
async function unzipJSZip(filename, outputDir, progressCallback) {
    //console.log(`ZipProxy.Zip.unzip.unzipJSZip() filename = '${filename}', outputDir = '${outputDir}'`);

    const fileEntry = await new Promise((resolve, reject) => {
        window.resolveLocalFileSystemURL(filename, resolve, reject);
    });

    const file = await new Promise((resolve, reject) => {
        fileEntry.file(resolve, reject);
    });

    const arrayBuffer = await new Promise((resolve, reject) => {
        const reader = new FileReader();
        reader.onloadend = () => resolve(reader.result);
        reader.onerror = reject;
        reader.readAsArrayBuffer(file);
    });

    const zip = new JSZip();
    const zipFiles = await zip.loadAsync(arrayBuffer);
    const fileEntries = Object.entries(zipFiles.files);
    let fileEntriesCompleted = 0;

    // First create all required directories
    for (const [fileName, zipEntry] of fileEntries) {
        if (zipEntry.dir) {
            await createDirectory(outputDir, fileName);
            if (progressCallback) {
                progressCallback(fileEntries.length, ++fileEntriesCompleted);
            }
        }
    }

    // Finally extract all files
    for (const [fileName, zipEntry] of fileEntries) {
        if (!zipEntry.dir) {
            await createFile(zipEntry, outputDir, fileName);
            if (progressCallback) {
                progressCallback(fileEntries.length, ++fileEntriesCompleted);
            }
        }
    }

    return true;
}

async function createDirectory(parentDir, dirName) {
    return new Promise((resolve, reject) => {
        window.resolveLocalFileSystemURL(parentDir, (parentDirEntry) => {
            parentDirEntry.getDirectory(
                dirName,
                { create: true, exclusive: false },
                resolve,
                (error) => {
                    if (error.code === FileError.PATH_EXISTS_ERR) {
                        resolve();
                    } else {
                        reject(error);
                    }
                }
            );
        }, reject);
    });
}

async function createFile(zipEntry, outputDir, fileName) {
    const content = await zipEntry.async('arraybuffer');

    const outputDirEntry = await new Promise((resolve, reject) => {
        window.resolveLocalFileSystemURL(outputDir, resolve, reject);
    });

    const fileWriter = await new Promise((resolve, reject) => {
        outputDirEntry.getFile(fileName, { create: true, exclusive: false }, resolve, reject);
    });

    return await new Promise((resolve, reject) => {
        fileWriter.createWriter((fileWriter) => {
            fileWriter.onwriteend = resolve;
            fileWriter.write(new Blob([content]));
        }, reject);
    });
}

cordova.commandProxy.add("Zip",
{
    unzip: function (successCallback, errorCallback, args)
    {
        if (!args || !args.length)
        {
            var message = "Error, something was wrong with the input filename. => " + filename;
            if (errorCallback)
                errorCallback(message);
            else
                console.error(message);
        }
		else
        {
            function progressCallback(total, loaded) {
                successCallback({loaded: loaded, total: total}, { keepCallback: true});
            }
	
            try {
                unzipJSZip(args[0], args[1], progressCallback)
                    .then(successCallback, errorCallback || console.error.bind(console));
            } catch (error) {
                errorCallback(error);
            }
        }
    }
});
