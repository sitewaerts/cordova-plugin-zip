// noinspection ES6ConvertVarToLetConst

var exec = cordova.require('cordova/exec');

/**
 *
 * @param result
 * @return {ProgressEvent}
 */
function newProgressEvent(result)
{
    return {
        loaded: result.loaded,
        total: result.total
    };
}

/**
 * @typedef {Object} ProgressEvent
 * @property {number} loaded
 * @property {number} total
 */

/**
 * The optional parameter algorithmName enables to select the zip algorithm under Windows platforms.
 * If algorithmName is undefined then index.js uses 'miniz-cpp' implementation if the
 * current platform supports it. Otherwise, it falls back to 'jszip'.
 * Supported algorithms:
 * 'jszip'       Default JavaScript implementation
 * 'miniz-cpp'   Native Windows 10 C/C++ UWP implementation (similar to iOS implementation)
 * 'andyzip'     Native Windows 10 C/C++ UWP implementation (faster for large files)
 *
 * @param {string|FileEntry} fileName
 * @param {string|FileEntry} outputDirectory
 * @param {(error?:any|number)=>void} callback
 * @param {(e:ProgressEvent)=>void} [progressCallback]
 * @param {string} [algorithmName]
 */
exports.unzip = function (fileName, outputDirectory, callback, progressCallback, algorithmName)
{
    var win = function (result)
    {
        if (result && typeof result.loaded != "undefined")
        {
            if (progressCallback)
            {
                return progressCallback(newProgressEvent(result));
            }
        }
        else if (callback)
        {
            callback(0);
        }
    };
    var fail = function (error)
    {
        if (callback)
        {
            callback(error);
        }
    };
    // TODO
    //   .nativeURL: null on electron
    //   .toURL(): iOS version of zip plugin cannot handle <scheme>:// urls (scheme from cordova config.xml)
    //   .toInternalURL():  old Windows version of zip plugin cannot handle cdv-file urls. fixed in version 3.2.11 of zip plugin, but this is not yet available in all deployed apps
    //   currently passing nativeURL for backwards compat. could possibly switch to .toInternalURL() when all Windows apps have been updated
    function toURL(file){
        if(!file)
            throw new Error("missing argument");
        if(typeof file === 'string')
            return file;
        return file.nativeURL || file.toURL();
    }
    cordova.exec(win, fail, "Zip", "unzip", [toURL(fileName), toURL(outputDirectory), algorithmName]);

    //exec(win, fail, 'Zip', 'unzip', [fileName, outputDirectory]);
};
