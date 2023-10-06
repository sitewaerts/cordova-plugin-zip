// package: decompress
// > npm install decompress --save
// NOTE: Has no progress indication
const decompress = require("decompress");

// package: extract-zip
// > npm install extract-zip --save
//const extract = require('extract-zip');

/**
 * @param {string} filename
 * @param {string} outputDir
 * @param {function(total:number, loaded:number):void} progressCallback
 * @return {PromiseLike<any>}
 */
async function unzipNode(filename, outputDir, progressCallback) {
    

    // Implementation for package: decompress
    /*
    const unzipPromise = await new Promise((resolve, reject) => {
        try {
            decompress(filename, outputDir)
                .then(files => { resolve(); })
                .catch(error => { reject(error); });
        } catch (error) {
            reject(error);
        }
    });
    */
    
    // Implementation for package: extract-zip
    /*
    const unzipPromise = await new Promise(async (resolve, reject) => {
        try {
            await extract(filename, {
                dir: outputDir,
                onEntry: (entry, zipfile) => {
                    if (progressCallback) {
                        progressCallback({ loaded: zipfile.entriesRead, total: zipfile.entryCount });
                    }
                }
            });
            resolve();
        } catch (error) {
            reject(error);
        }
    });
    */

    return await unzipPromise;
}

const logEnabled = true;

function log(message) {
    if (logEnabled !== true) {
        return;
    }
    console.log(`cordova-plugin-zip > src > electron > ZipProxy.js: ${message}`);
}

module.exports = {
    unzip: function(args)
    {
        // Why are the arguments packed like this (array inside array)?
        // And how can we access the success/error/progress callbacks passed into cordova.exec() in '../../zip.js' ?
        const fileName = args[0][0];
        const outputDirectory = args[0][1];

        log(`unzip: args = ${JSON.stringify(args)}`);
        log(`unzip: fileName = ${fileName}`);
        log(`unzip: outputDirectory = ${outputDirectory}`);
        
        try {
            decompress(fileName, outputDirectory)
                .then(files => { log(`unzip: success`); })
                .catch(error => { log(`unzip: error = ${error}`); });
            
            // TODO: Call success callback
            return true;
        } catch (error) {
            // TODO: Call error callback
            log(`unzip: catch(error) = ${error}`);
            return false;
        }
    }
};

