// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from path_resolver.djinni

#import <Foundation/Foundation.h>


/**
 * Module used to resolve file paths. libledger-core has its own iternal representation of the file system that may not
 * be adapted for the runtime platform. All path given to the PathResolver are absolute.
 */
@protocol LGPathResolver

/**
 * Resolves the path for a SQLite database file.
 * @param path The path to resolve.
 * @return The resolved path.
 */
- (nonnull NSString *)resolveDatabasePath:(nonnull NSString *)path;

/**
 * Resolves the path of a single log file.
 * @param path The path to resolve.
 * @return The resolved path.
 */
- (nonnull NSString *)resolveLogFilePath:(nonnull NSString *)path;

/**
 * Resolves the path for a json file.
 * @param path The path to resolve.
 * @return The resolved path.
 */
- (nonnull NSString *)resolvePreferencesPath:(nonnull NSString *)path;

@end