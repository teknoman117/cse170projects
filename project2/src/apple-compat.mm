#include <project2/directories.hpp>
#import <Foundation/Foundation.h>

std::string GetApplicationResourcesDirectory()
{
    CFURLRef appUrlRef = CFBundleCopyBundleURL( CFBundleGetMainBundle() );
    CFStringRef macPath = CFURLCopyFileSystemPath( appUrlRef, kCFURLPOSIXPathStyle );
    
    NSString *foo = (NSString *) macPath;
    std::string applicationDirectory([foo UTF8String]);
    
    CFRelease(appUrlRef);
    CFRelease(macPath);
    
    return applicationDirectory + "/Contents/Resources";
}