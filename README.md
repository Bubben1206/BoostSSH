# BoostSSH
Peer-To-Peer communication via SSH protocol, encrypted using 4096-bit RSA-keypair and AES128.

## Requirements

Server and Client requires boost::asio to compile! Boost version 1.8.2 is recommended.

Windows OS recommended, it might compile on Linux but I haven't tried it. Feel free to try it on Linux!


## How to use:

Download RSA folder and run "RSAKeyGen.exe" to generate a RSA keypair.

Copy the publickey file and privatekey file and paste the files into the Client folder.

Copy ONLY the publickey file and paste it into the Server folder.

Compile and run the Server files, this will open port 22 and wait for a connection.

Compile and run the client, then specify what IP Address you want to establish a connection with, then specify the port number (in this case 22). If everything works it should say "Auth success".

## Need help?

Join my discord and we can discuss your problem.

## Discord

If you have suggestions join my discord server
https://discord.gg/hDrDdascC5

