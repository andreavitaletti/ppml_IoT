import ecdsa
import hashlib
import binascii
import thread
import time
import sys
import os
import getopt
import serial

pkString = "543e98f814550813b51a1d0202d70eaba098746191123d9650fad594a286a8b0d07bda36ba8ed39aa016110e1b6e8113d7f423a1b29baff66bc42adfbde4615c"

vk = ecdsa.VerifyingKey.from_string(binascii.unhexlify(pkString), curve=ecdsa.NIST256p)

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

with open("goodDataSensSign.txt", 'r') as f:
	lineData = f.readlines()
	signature = ""
	data = ""
	hashing = ""

	for d in lineData:
		dataArray = d.split(" ")
		if "Data:" in dataArray[0]:
			data = d.replace("Data: ", "").replace(" ", "").strip()
			print "DATA: " + data
		if "Hash:" in dataArray[0]:
			hashing = d.replace("Hash: ", "").replace(" ", "").strip()
			print "HASH: " + hashing
		if "Signature:" in dataArray[0]:
			signature = d.replace("Signature: ", "").replace(" ", "").strip()
			print "SIGN: " + signature
			try:
				vk.verify_digest(binascii.unhexlify(signature), binascii.unhexlify(hashing)) # True
				print "\tVerification OK"
			except:
				print "\tVerification ERROR"
			print
