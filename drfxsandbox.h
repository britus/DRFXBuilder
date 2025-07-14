#ifndef DRFXSANDBOX_H
#define DRFXSANDBOX_H

extern "C" {
/**
 * @brief Get the bundle version
 * @return a string
 */
const char* GetBundleVersion();

/**
 * @brief Get the bundle build number
 * @return a string
 */
const char* GetBuildNumber();

/**
 * @brief Create a security scoped URL and saves the bookmark in the user default settings
 * @param fileName
 * @return Pointer to the security scoped URL or NULL if failed
 */
void* openFileBookmark(void* fileName);

/**
 * @brief Remove the given security scoped URL and does not remove the bookmark in the default user settings
 * @param Pointer to the security scoped URL
 */
void closeFileBookmark(void* securityScopedURL);
}

#endif // DRFXSANDBOX_H
