//#include <Foundation/Foundation.h>
#include <Foundation/NSData.h>
#include <Foundation/NSObject.h>
#include <Foundation/NSString.h>
#include <Foundation/NSURL.h>
#include <AppKit/NSOpenPanel.h>

extern "C" {

/**
 *
 */
const char* GetBundleVersion() {
    NSBundle *bundle = [NSBundle mainBundle];
    if (!bundle) return "Unknown";
    
    NSDictionary *infoDict = [bundle infoDictionary];
    NSString *version = [infoDict objectForKey:@"CFBundleVersion"];
    return version ? [version UTF8String] : "0.0";
}

/**
 *
 */
const char* GetBuildNumber() {
    NSBundle *bundle = [NSBundle mainBundle];
    if (!bundle) return "Unknown";
    
    NSDictionary *infoDict = [bundle infoDictionary];
    NSString *build = [infoDict objectForKey:@"CFBundleShortVersionString"];
    return build ? [build UTF8String] : "0";
}

/**
 *
 */
void* setSandboxBookmark(void* fileName)
{
    NSString* _fileName = (NSString*) fileName;
    if (_fileName) {
        NSLog(@"[OS-SANDBOX] Request for resource access:\n%@", _fileName);
        
        @autoreleasepool {
            NSURL *fileUrl = [NSURL URLWithString:_fileName];
            BOOL isStale = NO;
            NSError *error = nil;
            NSURL* outUrl = nil;
            
            // get the last bookmark for file name in user persistence data
            NSData* bookmark = [[NSUserDefaults standardUserDefaults] objectForKey:_fileName];
            if (bookmark) {
                outUrl = [NSURL URLByResolvingBookmarkData:bookmark
                                                   options:NSURLBookmarkResolutionWithSecurityScope
                                             relativeToURL:nil
                                       bookmarkDataIsStale:&isStale
                                                     error:&error];
            }
            // require new bookmark from file URL?
            if (!outUrl || !bookmark) {
                NSMutableArray *keys = [[NSMutableArray alloc] initWithCapacity:1];
                [keys addObject: _fileName];
                bookmark = [fileUrl bookmarkDataWithOptions:0
                             includingResourceValuesForKeys:keys
                                              relativeToURL:nil
                                                      error:&error];
                if (!bookmark) {
                    NSLog(@"[OS-SANDBOX] Unable to create security scope bookmark:\n%@", error.localizedDescription);
                    return nil;
                }
                
                // Resolve bookmark to security scoped URL
                outUrl = [NSURL URLByResolvingBookmarkData:bookmark
                                                   options:NSURLBookmarkResolutionWithoutUI
                                             relativeToURL:nil
                                       bookmarkDataIsStale:&isStale
                                                     error:&error];
                if (!outUrl) {
                    NSLog(@"[OS-SANDBOX] Unable to resolve security scoped bookmark:\n%@", error.localizedDescription);
                    return nil;
                }
            }
            // notice
            if (isStale) {
                NSLog(@"[OS-SANDBOX] Security scope bookmark is old. Please create a new one.");
            }
            // obtain security scoped access
            if ([outUrl startAccessingSecurityScopedResource]) {
                NSLog(@"[OS-SANDBOX] Access grated.");
                // Save to Sandbox bookmark trash bin - Happy claim nonsense storage space
                [[NSUserDefaults standardUserDefaults] setObject:bookmark forKey:_fileName];
                [[NSUserDefaults standardUserDefaults] synchronize];
                return outUrl;
            }
            else {
                NSLog(@"[OS-SANDBOX] Access denied!");
            }
        }
    }
    
    return NULL;
}

// close security access
void closeFileBookmark(void* fileURL)
{
    NSURL *_fileURL;
    if ((_fileURL = (NSURL*) fileURL)) {
        [_fileURL stopAccessingSecurityScopedResource];
    }
}

}
