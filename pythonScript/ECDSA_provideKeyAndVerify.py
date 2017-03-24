import ecdsa
import hashlib
import binascii
import thread
import time
import sys
import getopt
import serial

def generateKeyPair():
  sk = ecdsa.SigningKey.generate(curve=ecdsa.NIST256p)
  vk = sk.get_verifying_key()
  return (sk, vk)

def verifySignature(msg, sign, pk):
  h = hashlib.sha256()
  h.update(msg)
  try:
    pk.verify_digest(sign, h.digest())
    return True
  except:
    return False

def main():
  print "ECC signature for IoT"
  (PrivateKey, PublicKey) = generateKeyPair()
  print "Private Key:\t" + PrivateKey.to_string().encode("hex").upper()
  print "Public Key:\t" + PublicKey.to_string().encode("hex").upper()

  with serial.Serial('COM9', 9600, timeout=1) as ser:
    #send private key to the node
    ser.write(PrivateKey.to_string())
    ser.write("\0")
    time.sleep(1)
    
    #start receiving
    while True:
      newData = ser.readline()
      try:
        (msg, digest, sign) = newData.split(" ")
        #if verifySignature(msg, binascii.unhexlify(sign), PublicKey):
        try:
          PublicKey.verify_digest(binascii.unhexlify(sign.strip()), binascii.unhexlify(digest.strip()))
          print "Signature verified"
          print "MSG:\t" + msg
        except:
          print "Signature rejected"
          print "PACKET:\t" + newData
      except:
        if len(newData) > 0:
          print "ERROR parsing:\t" + newData.strip()

if __name__ == "__main__":
  main()
