const mongoose = require('mongoose');
const Schema = mongoose.Schema;

const logCamSchema = new Schema({
    noiDung: {
        type: String,
        required: true
    }, 
  }, { timestamps: true }
);

const LogCam = mongoose.model('LogCam', logCamSchema);
module.exports = LogCam;