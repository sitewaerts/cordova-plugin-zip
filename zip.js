var exec = cordova.require('cordova/exec');

function newProgressEvent(result) {
    var event = {
            loaded: result.loaded,
            total: result.total
    };
    return event;
}

// The optional parameter algorithmName enables to select the zip algorithm under Windows platforms.
// If algorithmName is undefined then ZipProxy.js uses 'miniz-cpp' implementation if the
// current platform supports it. Otherwise it falls back to 'jszip'.
// Supported algorithms:
// 'jszip'       Default JavaScript implementation
// 'miniz-cpp'   Native Windows 10 C/C++ UWP implementation (similar to iOS implementation)
// 'andyzip'     Native Windows 10 C/C++ UWP implementation (faster for large files)
exports.unzip = function(fileName, outputDirectory, callback, progressCallback, algorithmName) {
    var win = function(result) {
        if (result && typeof result.loaded != "undefined") {
            if (progressCallback) {
                return progressCallback(newProgressEvent(result));
            }
        } else if (callback) {
            callback(0);
        }
    };
    var fail = function(error) {
        if (callback) {
            callback(error);
        }
    };
    cordova.exec(win, fail, "Zip", "unzip", [fileName, outputDirectory, algorithmName]);

    //exec(win, fail, 'Zip', 'unzip', [fileName, outputDirectory]);
};
