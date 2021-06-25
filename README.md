# cordova-plugin-zip

A Cordova plugin to unzip files on Android, iOS and Windows 10.

## Installation

    cordova plugin add cordova-plugin-zip

## Usage

    zip.unzip(<source zip>, <destination dir>, <callback>, [<progressCallback>], [<unzipAlgorithm>]);

Both source and destination arguments can be URLs obtained from the HTML File
interface or absolute paths to files on the device.

The callback argument will be executed when the unzip is complete, or when an
error occurs. It will be called with a single argument, which will be 0 on
success, or -1 on failure.

The progressCallback argument is optional and will be executed whenever a new ZipEntry
has been extracted. E.g.:

    var progressCallback = function(progressEvent) {
        $( "#progressbar" ).progressbar("value", Math.round((progressEvent.loaded / progressEvent.total) * 100));
    };

The values `loaded` and `total` are the number of compressed bytes processed and total. Total is the
file size of the zip file.

The unzipAlgorithm argument is optional and only evaluated under windows.
Possible options are:
* 'jszip' (JavaScript implementation jszip)
* 'miniz-cpp' (Native Windows 10 C/C++ UWP implementation similar to iOS implementation)
* 'andyzip' (Native Windows 10 C/C++ UWP implementation slightly faster than miniz-cpp)

If unzipAlgorithm is undefined, the plugin will default to 'andyzip'.
The 'andyzip' implementation offsers only basic zip format support with 'deflate' and 'uncompressed'
as the only supported decoding formats for archive contents.
The 'miniz-cpp' implementation is more robust and should be preferred for questinable archive sources.
Both native implementations are more than 10 times faster than the jszip implementation.


## Release Notes

### 3.2.0 (Jun 25, 2021)
* Added native C/C++ implementations for windows based on miniz-cpp and andyzip

### 3.1.2 (Feb 23, 2021)
* progress callback for windows
* jszip update

### 3.1.1 (Jun 13, 2020)
* several bugfixes for windows path handling

### 3.1.0 (Feb 23, 2016)
* Updated SSZipArchive (ios lib) to 1.1

### 3.0.0 (May 1, 2015)
* Updated SSZipArchive (ios lib) to 0.2.1
* Update file plugin dependency to use npm version (cordova-plugin-file)

### 2.1.0 (May 1, 2014)
* Added progress events
* Fix iOS path handling when given file: URLs as src/target
