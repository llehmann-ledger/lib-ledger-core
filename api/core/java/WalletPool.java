// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from wallet_pool.djinni

package co.ledger.core;

import java.util.concurrent.atomic.AtomicBoolean;

public abstract class WalletPool {
    public abstract void getOrCreateBitcoinLikeWallet(BitcoinLikeExtendedPublicKeyProvider publicKeyProvider, BitcoinLikeNetworkParameters networkParams, DynamicObject configuration, BitcoinLikeWalletCallback callback);

    public abstract void getBitcoinLikeWallet(String identifier, BitcoinLikeWalletCallback callback);

    public abstract void getSupportedBitcoinLikeNetworkParameters(BitcoinLikeNetworkParametersCallback callback);

    public abstract void addBitcoinLikeNetworkParameters(BitcoinLikeNetworkParameters params);

    public abstract void removeBitcoinLikenetworkParameters(BitcoinLikeNetworkParameters params);

    public abstract Logger getLogger();

    public abstract Preferences getPreferences();

    public abstract Preferences getWalletPreferences(String walletIdentifier);

    public abstract Preferences getAccountPreferences(String walletIdentifier, int accountNumber);

    public abstract Preferences getOperationPreferences(String uid);

    public abstract WalletListCallback getWallets();

    private static final class CppProxy extends WalletPool
    {
        private final long nativeRef;
        private final AtomicBoolean destroyed = new AtomicBoolean(false);

        private CppProxy(long nativeRef)
        {
            if (nativeRef == 0) throw new RuntimeException("nativeRef is zero");
            this.nativeRef = nativeRef;
        }

        private native void nativeDestroy(long nativeRef);
        public void destroy()
        {
            boolean destroyed = this.destroyed.getAndSet(true);
            if (!destroyed) nativeDestroy(this.nativeRef);
        }
        protected void finalize() throws java.lang.Throwable
        {
            destroy();
            super.finalize();
        }

        @Override
        public void getOrCreateBitcoinLikeWallet(BitcoinLikeExtendedPublicKeyProvider publicKeyProvider, BitcoinLikeNetworkParameters networkParams, DynamicObject configuration, BitcoinLikeWalletCallback callback)
        {
            assert !this.destroyed.get() : "trying to use a destroyed object";
            native_getOrCreateBitcoinLikeWallet(this.nativeRef, publicKeyProvider, networkParams, configuration, callback);
        }
        private native void native_getOrCreateBitcoinLikeWallet(long _nativeRef, BitcoinLikeExtendedPublicKeyProvider publicKeyProvider, BitcoinLikeNetworkParameters networkParams, DynamicObject configuration, BitcoinLikeWalletCallback callback);

        @Override
        public void getBitcoinLikeWallet(String identifier, BitcoinLikeWalletCallback callback)
        {
            assert !this.destroyed.get() : "trying to use a destroyed object";
            native_getBitcoinLikeWallet(this.nativeRef, identifier, callback);
        }
        private native void native_getBitcoinLikeWallet(long _nativeRef, String identifier, BitcoinLikeWalletCallback callback);

        @Override
        public void getSupportedBitcoinLikeNetworkParameters(BitcoinLikeNetworkParametersCallback callback)
        {
            assert !this.destroyed.get() : "trying to use a destroyed object";
            native_getSupportedBitcoinLikeNetworkParameters(this.nativeRef, callback);
        }
        private native void native_getSupportedBitcoinLikeNetworkParameters(long _nativeRef, BitcoinLikeNetworkParametersCallback callback);

        @Override
        public void addBitcoinLikeNetworkParameters(BitcoinLikeNetworkParameters params)
        {
            assert !this.destroyed.get() : "trying to use a destroyed object";
            native_addBitcoinLikeNetworkParameters(this.nativeRef, params);
        }
        private native void native_addBitcoinLikeNetworkParameters(long _nativeRef, BitcoinLikeNetworkParameters params);

        @Override
        public void removeBitcoinLikenetworkParameters(BitcoinLikeNetworkParameters params)
        {
            assert !this.destroyed.get() : "trying to use a destroyed object";
            native_removeBitcoinLikenetworkParameters(this.nativeRef, params);
        }
        private native void native_removeBitcoinLikenetworkParameters(long _nativeRef, BitcoinLikeNetworkParameters params);

        @Override
        public Logger getLogger()
        {
            assert !this.destroyed.get() : "trying to use a destroyed object";
            return native_getLogger(this.nativeRef);
        }
        private native Logger native_getLogger(long _nativeRef);

        @Override
        public Preferences getPreferences()
        {
            assert !this.destroyed.get() : "trying to use a destroyed object";
            return native_getPreferences(this.nativeRef);
        }
        private native Preferences native_getPreferences(long _nativeRef);

        @Override
        public Preferences getWalletPreferences(String walletIdentifier)
        {
            assert !this.destroyed.get() : "trying to use a destroyed object";
            return native_getWalletPreferences(this.nativeRef, walletIdentifier);
        }
        private native Preferences native_getWalletPreferences(long _nativeRef, String walletIdentifier);

        @Override
        public Preferences getAccountPreferences(String walletIdentifier, int accountNumber)
        {
            assert !this.destroyed.get() : "trying to use a destroyed object";
            return native_getAccountPreferences(this.nativeRef, walletIdentifier, accountNumber);
        }
        private native Preferences native_getAccountPreferences(long _nativeRef, String walletIdentifier, int accountNumber);

        @Override
        public Preferences getOperationPreferences(String uid)
        {
            assert !this.destroyed.get() : "trying to use a destroyed object";
            return native_getOperationPreferences(this.nativeRef, uid);
        }
        private native Preferences native_getOperationPreferences(long _nativeRef, String uid);

        @Override
        public WalletListCallback getWallets()
        {
            assert !this.destroyed.get() : "trying to use a destroyed object";
            return native_getWallets(this.nativeRef);
        }
        private native WalletListCallback native_getWallets(long _nativeRef);
    }
}
