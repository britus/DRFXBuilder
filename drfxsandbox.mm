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

NSData* getBookmark(NSURL *securityScopedURL, NSString*)
{
    @autoreleasepool {
        NSOpenPanel *panel = [NSOpenPanel openPanel];
        [panel setCanChooseFiles:YES];
        [panel setCanChooseDirectories:YES];
        [panel setAllowsMultipleSelection:NO];
        [panel setDirectoryURL:securityScopedURL];
        [panel setTitle:@"Stupid! Sandbox File Selector"];
        [panel setPrompt:@"Select"];
        [panel setMessage:@"Sorry for that Sandbox SHIT! YOU MUST CREATE SECURITY BOOKMARKS FIRST!"];

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
                    NSLog(@"Bookmark erfolgreich erzeugt.");

                    // Beispiel: Bookmark in UserDefaults speichern
                    [[NSUserDefaults standardUserDefaults] setObject:bookmark forKey:@"MySecurityScopedBookmark"];
                    [[NSUserDefaults standardUserDefaults] synchronize];
                    return bookmark;
                } else {
                    NSLog(@"Fehler beim Erzeugen des Bookmarks: %@", error);
                }
            }
        } else {
            NSLog(@"Benutzer hat keine Datei ausgewählt.");
        }
    }
    return NULL;
}
void* openFileBookmark(void* fileName)
{
    NSString* _fileName = (NSString*) fileName;
    if (_fileName) {
        @autoreleasepool {
            BOOL isStale = NO;
            NSError *error = nil;
            NSURL *fileUrl = [NSURL URLWithString:_fileName];
            NSData *bookmark = nil;

            // find in user persistence data
            NSURL* outUrl = [NSURL URLByResolvingBookmarkData:[[NSUserDefaults standardUserDefaults]
                                                               objectForKey:_fileName]
                                                      options:NSURLBookmarkResolutionWithSecurityScope
                                                relativeToURL:nil
                                          bookmarkDataIsStale:&isStale
                                                        error:&error];
            if (!outUrl) {
                // new bookmark from URL
                bookmark = [fileUrl bookmarkDataWithOptions:0
                             includingResourceValuesForKeys:nil
                                              relativeToURL:nil
                                                      error:&error];
                if (!bookmark) {
                    NSLog(@"Unable to create security scope bookmarks: %@", error.localizedDescription);
                    return nil;
                }

                // Lesezeichen wieder in eine URL auflösen
                outUrl = [NSURL URLByResolvingBookmarkData:bookmark
                                                   options:NSURLBookmarkResolutionWithoutUI
                                             relativeToURL:nil
                                       bookmarkDataIsStale:&isStale
                                                     error:&error];
                if (!outUrl) {
                    NSLog(@"Unable to resolve security scope bookmarks: %@", error.localizedDescription);
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
void closeFileBookmark(void* securityScopedURL)
{
    NSURL *_securityScopedURL;
    if ((_securityScopedURL = (NSURL*) securityScopedURL)) {
        [_securityScopedURL stopAccessingSecurityScopedResource];
    }
}
}
