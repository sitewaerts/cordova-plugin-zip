const logEnabled = false;

function log(message) {
    if (logEnabled !== true) {
        return;
    }
    console.log(`cordova-plugin-zip > src > electron > ZipProxy.js: ${message}`);
}


/**
 * Implementation using package: decompress
 * > npm install decompress --save
 * NOTE: Doesn't support progress callbacks so we don't use this implementation but keep it as a reference for testing.
 * 
 * @param {string} filename
 * @param {string} outputDir
 * @param {function(total:number, loaded:number):void} progressCallback
 * @return {PromiseLike<any>}
 */
/*
const decompress = require("decompress");
async function unzipWithDecompress(filename, outputDir, progressCallback) {
    log(`unzipWithDecompress = ${filename} -> ${outputDir}, progressCallback = ${progressCallback}`);
    return decompress(filename, outputDir)
        .then(files => { resolve(); })
        .catch(error => { reject(error); });
}
*/


/**
 * Implementation using package: extract-zip
 * > npm install extract-zip --save
 * 
 * @param {string} filename
 * @param {string} outputDir
 * @param {function(total:number, loaded:number):void} progressCallback
 * @return {PromiseLike<any>}
 */
const extract = require('extract-zip');
async function unzipWithExtractZip(filename, outputDir, progressCallback) {
    log(`unzipWithExtractZip = ${filename} -> ${outputDir}, progressCallback = ${progressCallback}`);
    return extract(filename, {
        dir: outputDir,
        onEntry: (entry, zipfile) => {
            log(`unzipWithExtractZip: entry = ${entry.fileName}`);
            if (progressCallback) {
                progressCallback({ loaded: zipfile.entriesRead, total: zipfile.entryCount });
            }
        }
    });
}


module.exports = {
    /**
     * @param {[fileName: String, outputDirectory: String]} args
     *      fileName - the full path of the zip archive
     *      outputDirectory - the full path of the directory where to extract the zip contents
     * @returns {Promise<void>} resolves when everything has been extracted successfully
     */
    unzip: function([args])
    {
        const fileName = args[0];
        const outputDirectory = args[1];
        const progressCallback = ({ total, loaded }) => {
            // TODO: Dummy progress callback until we have a solution for the issue that the electron implementation
            // doesn't offer a way to use them.
            log(`unzip: progressCallback(total: ${total}, loaded: ${loaded})`);
        };

        log(`unzip: args = ${JSON.stringify(args)}`);
        log(`unzip: fileName = ${fileName}`);
        log(`unzip: outputDirectory = ${outputDirectory}`);
        
        return unzipWithExtractZip(fileName, outputDirectory, progressCallback)
    }
};

