// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from configuration.djinni

package co.ledger.core;

import java.util.concurrent.atomic.AtomicBoolean;

public abstract class Configuration {
    /** Selects the keychain engine (P2PKH, P2SH...) */
    public static final String KEYCHAIN_ENGINE = "KEYCHAIN_ENGINE";

    /** Sets the derivation scheme for the KEYCHAIN (defaults 44'/<coin_type>'/<account>'/<node>/<address>) */
    public static final String KEYCHAIN_DERIVATION_SCHEME = "KEYCHAIN_DERIVATION_SCHEME";

    /** Sets the observable range for HD keychains (BIP32 based) */
    public static final String KEYCHAIN_OBSERVABLE_RANGE = "KEYCHAIN_OBSERVABLE_RANGE";

    /** Selects the blockchain explorer engine (Ledger's API, Electrum server, RPC) */
    public static final String BLOCKCHAIN_EXPLORER_ENGINE = "BLOCKCHAIN_EXPLORER_ENGINE";

    /** Sets the API endpoint for API based engine (Ledger's API) */
    public static final String BLOCKCHAIN_EXPLORER_API_ENDPOINT = "BLOCKCHAIN_EXPLORER_API_ENDPOINT";

    /** Selects the blockchain observer engine (Ledger's API) */
    public static final String BLOCKCHAIN_OBSERVER_ENGINE = "BLOCKCHAIN_OBSERVER_ENGINE";

    /** Selects the synchronization engine */
    public static final String SYNCHRONIZATION_ENGINE = "SYNCHRONIZATION_ENGINE";


    private static final class CppProxy extends Configuration
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
    }
}
