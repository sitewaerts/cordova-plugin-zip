// Type definitions for Apache Cordova Zip plugin
// Project: https://github.com/sitewaerts/cordova-plugin-zip
// Definitions by: Microsoft Open Technologies Inc. <http://msopentech.com>
// Definitions: https://github.com/DefinitelyTyped/DefinitelyTyped
//
// Copyright (c) Microsoft Open Technologies Inc
// Licensed under the MIT license

type ZipAlgorithm =  'jszip' |  'miniz-cpp' |  'andyzip'

interface ZipPlugin {

    unzip(sourceZip:string, destinationDir:string, callback:(error:0|any)=>void, progressCallback?:(progress:{loaded:number,total:number})=>void, unzipAlgorithm?:ZipAlgorithm)

}

interface Window {
    zip: ZipPlugin
}
