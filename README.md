# cordova-plugin-zip

A Cordova plugin to unzip files on Android, iOS and Windows 10.

## Installation

    cordova plugin add https://github.com/sitewaerts/cordova-plugin-zip

## Usage

    zip.unzip(<source zip>, <destination dir>, <callback>, [<progressCallback>], [<unzipAlgorithm>]);

Both source and destination arguments can be URLs obtained from the HTML File
interface or absolute paths to files on the device.

The callback argument will be executed when unzip is complete, or when an
error occurs. It will be called with a single argument, which will be 0 on
success, or an error object on failure.

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

If unzipAlgorithm is undefined, the plugin will default to 'miniz-cpp'.
The 'miniz-cpp' implementation is more robust and handles various zip formats more reliable.
The 'andyzip' implementation offsers only basic zip format support with 'deflate' and 'uncompressed'
as the only supported decoding formats for archive contents, but tends to be slightly faster.
Both native implementations are more than 10 times faster than the jszip implementation.
Note that 'jszip' is the only algorithm that doesn't extract empty folders.


## Release Notes

### 3.2.6 (Oct 07, 2022)
* Bugfix for Fixed zip path traversal vulnerability in Android implementation (Zip.java)

### 3.2.5 (Sep 29, 2022)
* Fixed zip path traversal vulnerability in Android implementation (Zip.java)

### 3.2.4 (Dec 14, 2021)
* Fixed miniz-cpp not extracting more than one file successfully

### 3.2.3 (Dec 8, 2021)
* Fixed miniz-cpp returning error when unzipping empty files
* Fixed andyzip crashing when unzipping invalid archives

### 3.2.2 (Jul 19, 2021)
* Fixed miniz-cpp not creating folders
* Improved caching for string operations and general error handling

### 3.2.1 (Jul 15, 2021)
* Changed default zip algorithm to miniz-cpp (more reliable and faster for many small files)
* Improved error catching and handling in ZipComponentUWP and ZipProxy.js
* Added version number to ZipComponentUWP and pre-build script that updates it

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
