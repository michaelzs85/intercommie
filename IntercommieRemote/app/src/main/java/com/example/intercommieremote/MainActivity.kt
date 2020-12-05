package com.example.intercommieremote

import android.content.Context
import android.net.http.SslError
import android.os.Bundle
import android.util.Log
import android.webkit.SslErrorHandler
import android.webkit.WebResourceRequest
import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.appcompat.app.AppCompatActivity
import java.io.*
import java.lang.reflect.Field
import java.security.KeyStore
import java.security.KeyStoreException
import java.security.NoSuchAlgorithmException
import java.security.cert.CertificateException
import javax.net.ssl.TrustManagerFactory
import javax.net.ssl.X509TrustManager

val TAG = "###"


@Throws(
    CertificateException::class,
    FileNotFoundException::class,
    IOException::class,
    KeyStoreException::class,
    NoSuchAlgorithmException::class
)
fun initTrustStore(context: Context) : TrustManagerFactory {
    // Load a KeyStore containing our trusted CAs
    val keyStoreType: String = KeyStore.getDefaultType()
    val trustedKeyStore: KeyStore = KeyStore.getInstance(keyStoreType)

    val fis = context.resources.openRawResource(R.raw.trusted_root)
    trustedKeyStore.load(fis, "yFD21CvcYm".toCharArray())

    // Create a TrustManager that trusts the CAs in our KeyStore
    val tmfAlgorithm: String = TrustManagerFactory.getDefaultAlgorithm()
    var tmf: TrustManagerFactory = TrustManagerFactory.getInstance(tmfAlgorithm)
    tmf.init(trustedKeyStore)
    return tmf
}

class MainActivity : AppCompatActivity() {

    private val grazUpcIp = "https://84.114.113.52:443"

    private class MyWebViewClient(context: Context) : WebViewClient() {

        private val tmf : TrustManagerFactory = initTrustStore(context)

        override fun shouldOverrideUrlLoading(
            view: WebView?,
            request: WebResourceRequest?
        ): Boolean {
            return false
        }

        override fun onReceivedSslError(view: WebView?, handler: SslErrorHandler, error: SslError) {
            Log.d(TAG, "onReceivedSslError")
            var passVerify = false
            if (error.primaryError == SslError.SSL_UNTRUSTED) {
                val cert = error.certificate
                val subjectDN = cert.issuedTo.dName
                Log.d(TAG, "subjectDN: $subjectDN")
                try {
                    val f: Field = cert.javaClass.getDeclaredField("mX509Certificate")
                    f.isAccessible = true

                    for (trustManager in tmf.getTrustManagers()) {
                        if (trustManager is X509TrustManager) {
                            val x509TrustManager: X509TrustManager = trustManager
                            try {
                                x509TrustManager.checkServerTrusted(arrayOf(cert.x509Certificate), "RSA")
                                passVerify = true
                                break
                            } catch (e: Exception) {
                                Log.e(TAG, "verify trustManager failed", e)
                                passVerify = false
                            }
                        }
                    }
                    Log.d(TAG, "passVerify: $passVerify")
                } catch (e: Exception) {
                    Log.e(TAG, "verify cert fail", e)
                }
            }
            if (passVerify) handler.proceed() else handler.cancel()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val webv = findViewById<WebView>(R.id.mainWebview)
        webv.settings.setJavaScriptEnabled(true)

        val myWebViewClient = MyWebViewClient(applicationContext)
        webv.webViewClient = myWebViewClient
        webv.loadUrl(grazUpcIp)

    }

}
