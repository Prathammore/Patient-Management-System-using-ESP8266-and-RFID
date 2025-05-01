const crypto = require("crypto");

const { publicKey, privateKey } = crypto.generateKeyPairSync("rsa", {
  modulusLength: 2048,
});


function encryptUID(uid) {
  
  const aesKey = crypto.randomBytes(32); // AES key for encryption
  

  const iv = crypto.randomBytes(16);

  
  const cipher = crypto.createCipheriv('aes-256-cbc', aesKey, iv);

  
  let encryptedUID = cipher.update(uid, 'utf8', 'hex');
  encryptedUID += cipher.final('hex');


  const encryptedAESKey = crypto.publicEncrypt(publicKey, aesKey);

  
  return {
    encryptedUID: encryptedUID,
    encryptedAESKey: encryptedAESKey.toString('base64'),
    iv: iv.toString('hex')  // Store the IV for decryption
  };
}


function decryptUID(encryptedUID, encryptedAESKey, iv) {
  
  const aesKey = crypto.privateDecrypt(privateKey, Buffer.from(encryptedAESKey, 'base64'));

  
  const decipher = crypto.createDecipheriv('aes-256-cbc', aesKey, Buffer.from(iv, 'hex'));

  
  let decryptedUID = decipher.update(encryptedUID, 'hex', 'utf8');
  decryptedUID += decipher.final('utf8');

  return decryptedUID;
}

module.exports = { encryptUID, decryptUID };
