# Importan commands for this project

## Root Certificate -> BKS key store

Prerequisits:
* jre
* Bouncy Castle Provider: http://www.bouncycastle.org

```
keytool.exe -import -v -trustcacerts -alias 0 -file .\ca_cer.pem -keystore "trusted_root.bks" -storetype BKS -provider org.bouncycastle.jce.provider.BouncyCastleProvider -providerpath .\bcprov-jdk15on-167.jar -storepass yFD21CvcYm
```
How To found here: http://blog.craz:ybob.org/2010/02/android-trusting-ssl-certificates.html

