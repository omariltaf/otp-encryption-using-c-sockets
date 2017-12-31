# otp-encryption-using-c-sockets
One-Time Pad Encryption using C socket programming.

**To compile run bash script:**
```
./compileall
```
**Generating a key:**
```
keygen [keylength] > [outputfile]
keygen 256 > keyfile
```

**Starting the encryption and decryption servers:**
```
otp_enc_d [portnumber] & 
otp_dec_d [portnumber] &
otp_enc_d 50761 &
otp_dec_d 50762 &
```
- The servers must be run in the background, so include an "&" at the end of each line.
- The encryption and decryption servers must use different ports.

**Encrypting a file:**
```
otp_enc [plaintextfile] [keyfile] [portnumber] > [ciphertextfile]
otp_enc plaintext1 keyfile 50761 > ciphertext1
```

**Decrypting a file:**
```
otp_dec [ciphertext] [keyfile] [portnumber] > [plaintextfile]
otp_dec ciphertext1 keyfile 50762 > newplaintext1
```
