const logEnabled = false;
const logPrefix = 'cordova-plugin-zip > src > electron > index.js';

function log(message) {
    if (logEnabled !== true) {
        return;
    }
    console.log(`${logPrefix}: ${message}`);
}


/**
 * Implementation using package: decompress
 * > npm install decompress --save
 * NOTE: Doesn't support progress callbacks, so we don't use this implementation but keep it as a reference for testing.
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


const zipPlugin = {

    /**
     * @param {string} fileName the full path of the zip archive
     * @param {string} outputDirectory the full path of the directory where to extract the zip contents
     * @param {string} algorithm
     * @param {CallbackContext} callbackContext
     * @void
     *
     */
    unzip: function([fileName, outputDirectory, algorithm], callbackContext)
    {
        const filePluginUtil = callbackContext.getCordovaService('File').util;
        const nativeFileName = filePluginUtil.urlToFilePath(fileName);
        const nativeOutputDirectory = filePluginUtil.urlToFilePath(outputDirectory);

        log(`unzip: fileName = ${fileName} -> ${nativeFileName}`);
        log(`unzip: outputDirectory = ${outputDirectory} -> ${nativeOutputDirectory}`);
        log(`unzip: algorithm = ${algorithm}`);

        const progressCallback = ({ total, loaded }) => {
                callbackContext.progress({ total, loaded });
        };
        unzipWithExtractZip(nativeFileName, nativeOutputDirectory, progressCallback)
            .then(() => {
                log(`unzip: succeeded`);
                callbackContext.success();
            })
            .catch((error) => {
                callbackContext.error(error);
            });        
    }
}

/**
 * cordova electron plugin api
 * @param {string} action
 * @param {Array<any>} args
 * @param {CallbackContext} callbackContext
 * @returns {boolean} indicating if action is available in plugin
 */
const plugin = function (action, args, callbackContext)
{
    if (!zipPlugin[action]) {
        log(`plugin-zip: unknown action = ${action}`);
        return false;
    }

    try {
        zipPlugin[action](args, callbackContext);
    } catch (e) {
        console.error(action + ' failed', e);
        callbackContext.error({message: action + ' failed', cause: e});
    }
    return true;
}

// backwards compatibility: attach api methods for direct access from old cordova-electron platform impl
Object.keys(zipPlugin).forEach((apiMethod) =>
{
    plugin[apiMethod] = (args) =>
    {
        return Promise.resolve((resolve, reject) =>
        {
            zipPlugin[apiMethod](args, {
                progress: (data) =>
                {
                    console.warn("cordova-plugin-zip: ignoring progress event as not supported in old plugin API", data);
                },
                success: (data) =>
                {
                    resolve(data)
                },
                error: (data) =>
                {
                    reject(data)
                }
            });
        });
    }
});


module.exports = plugin;
