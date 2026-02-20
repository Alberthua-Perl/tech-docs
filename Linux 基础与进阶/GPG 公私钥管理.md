# GPG 公私钥的管理

```bash
I need you to create a gpg key.
1) create a gpg key
    gpg −−gen−key
    default ok
    default ok
    default ok
    y (confirm)
    Your real name
    your WORK email address (i.e. *this* email)
    comment optional or blank

After key is created, you can view it:
    gpg −−list−keys  列出公钥
    gpg −−list−sigs (later after it has been signed)

then upload it:
    gpg −−send−key <key id> 上传公钥

for <key id> use the email, or the hex id.

if your system is not setup properly, then you may need to supply a server.
    gpg −−keyserver pgp.mit.edu −−send−keys <key id>  或者 subkeys.pgp.net

Once your gpg key is uploaded, send me the finger print
    gpg −−list−key −−fingerprint

I will then sign your key, and send you an ascii hex file with the signed key.
When you get the email:
1) Saved signed key to file
2) import signed key to key ring
    gpg −−import <file>

3) upload signed key
    gpg −−send−key <ID>

Email me that the signed key is uploaded. I will verify, and only then can I start your CX training.
```