//#include <Foundation/Foundation.h>
#include <Foundation/NSData.h>
#include <Foundation/NSObject.h>
#include <Foundation/NSString.h>
#include <Foundation/NSURL.h>
#include <AppKit/NSOpenPanel.h>

extern "C" {
const char* GetBundleVersion() {
    NSBundle *bundle = [NSBundle mainBundle];
    if (!bundle) return "Unknown";

    NSDictionary *infoDict = [bundle infoDictionary];
    NSString *version = [infoDict objectForKey:@"CFBundleVersion"];
    return version ? [version UTF8String] : "0.0";
}

const char* GetBuildNumber() {
    NSBundle *bundle = [NSBundle mainBundle];
    if (!bundle) return "Unknown";

    NSDictionary *infoDict = [bundle infoDictionary];
    NSString *build = [infoDict objectForKey:@"CFBundleShortVersionString"];
    return build ? [build UTF8String] : "0";
}

NSData* getBookmark(NSURL *fileURL, NSString*)
{
    @autoreleasepool {
        NSString* m1 = @"Stupid! Sandbox File Selector";
        NSString* m2 = @"Select";
        NSString* m3 = @"YOU MUST CREATE SECURITY BOOKMARKS FIRST!";
        NSOpenPanel *panel = [NSOpenPanel openPanel];
        [panel setCanChooseFiles:YES];
        [panel setCanChooseDirectories:YES];
        [panel setAllowsMultipleSelection:NO];
        [panel setDirectoryURL:fileURL];
        [panel setTitle:m1.localizedCapitalizedString];
        [panel setPrompt:m2.localizedCapitalizedString];
        [panel setMessage:m3.localizedCapitalizedString];

        if ([panel runModal] == NSModalResponseOK) {
            NSURL *selectedURL = [[panel URLs] firstObject];
            if (selectedURL) {
                NSError *error = nil;
                NSData *bookmark = [selectedURL bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope
                                                includingResourceValuesForKeys:nil
                                                relativeToURL:nil
                                                error:&error];
                if (bookmark) {
                    // Bookmark erfolgreich erstellt
                    NSLog(@"Bookmark successfully built.");
                    [[NSUserDefaults standardUserDefaults] setObject:bookmark forKey:selectedURL.absoluteString];
                    [[NSUserDefaults standardUserDefaults] synchronize];
                    return bookmark;
                } else {
                    NSLog(@"Could not create sercurity bookmarks: %@", error.localizedDescription);
                }
            }
        }
    }
    return NULL;
}
void* openFileBookmark(void* fileName)
{
    NSString* _fileName = (NSString*) fileName;
    if (_fileName) {
        NSLog(@"[OS-SANDBOX] Request for file or path access: %@", _fileName);

        @autoreleasepool {
            NSURL *fileUrl = [NSURL URLWithString:_fileName];
            BOOL isStale = NO;
            NSError *error = nil;
            NSData *bookmark = nil;
            // get the last bookmark for file name in user persistence data
            NSData* _bookmark = [[NSUserDefaults standardUserDefaults] objectForKey:_fileName];
            NSURL* outUrl = nil;

            if (_bookmark) {
                outUrl = [NSURL URLByResolvingBookmarkData:_bookmark
                                                          options:NSURLBookmarkResolutionWithSecurityScope
                                                    relativeToURL:nil
                                              bookmarkDataIsStale:&isStale
                                                            error:&error];
            }

            if (!outUrl) { // new bookmark from file URL
                bookmark = [fileUrl bookmarkDataWithOptions:0
                             includingResourceValuesForKeys:nil
                                              relativeToURL:nil
                                                      error:&error];
                if (!bookmark) {
                    NSLog(@"Unable to create security scope bookmarks: %@", error.localizedDescription);
                    return nil;
                }

                // Resolve bookmark to security scoped URL
                outUrl = [NSURL URLByResolvingBookmarkData:bookmark
                                                   options:NSURLBookmarkResolutionWithoutUI
                                             relativeToURL:nil
                                       bookmarkDataIsStale:&isStale
                                                     error:&error];
                if (!outUrl) {
                    NSLog(@"Unable to resolve security scoped bookmarks: %@", error.localizedDescription);
                    return nil;
                }

                [[NSUserDefaults standardUserDefaults] setObject:bookmark forKey:_fileName];
                [[NSUserDefaults standardUserDefaults] synchronize];
            }

            if (isStale) {
                NSLog(@"Security scope bookmarks is old. Please create a new one.");
            }

            if ([outUrl startAccessingSecurityScopedResource]) {
                NSLog(@"Access grated to security scoped resource.");
                return outUrl;
            } else {
                NSLog(@"Unable to access security scoped resource. Open sucking panel!!!");
                if ((bookmark = getBookmark(fileUrl, _fileName))) {
                    outUrl = [NSURL URLByResolvingBookmarkData:bookmark
                                        options:NSURLBookmarkResolutionWithSecurityScope
                                  relativeToURL:nil
                            bookmarkDataIsStale:&isStale
                                          error:&error];
                    if ([outUrl startAccessingSecurityScopedResource]) {
                        NSLog(@"Access grated to security scoped resource.");
                        [[NSUserDefaults standardUserDefaults] setObject:bookmark forKey:_fileName];
                        [[NSUserDefaults standardUserDefaults] synchronize];
                        return outUrl;
                    } else {
                        NSLog(@"Unable to access security scoped resource.");
                    }
                }
            }
        }
    }
    return 0;
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
