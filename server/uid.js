let currentUID = null;

module.exports = {
  setUID: (uid) => {
    currentUID = uid;
  },
  getUID: () => currentUID,
  clearUID: () => {
    currentUID = null;
  }
};
