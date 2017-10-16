// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from wallet.djinni

#import <Foundation/Foundation.h>

@interface LGBlock : NSObject
- (nonnull instancetype)initWithHash:(nonnull NSString *)hash
                                 uid:(nonnull NSString *)uid
                                time:(nonnull NSDate *)time
                        currencyName:(nonnull NSString *)currencyName
                              height:(int64_t)height;
+ (nonnull instancetype)BlockWithHash:(nonnull NSString *)hash
                                  uid:(nonnull NSString *)uid
                                 time:(nonnull NSDate *)time
                         currencyName:(nonnull NSString *)currencyName
                               height:(int64_t)height;

@property (nonatomic, readonly, nonnull) NSString * hash;

@property (nonatomic, readonly, nonnull) NSString * uid;

@property (nonatomic, readonly, nonnull) NSDate * time;

@property (nonatomic, readonly, nonnull) NSString * currencyName;

@property (nonatomic, readonly) int64_t height;

@end
