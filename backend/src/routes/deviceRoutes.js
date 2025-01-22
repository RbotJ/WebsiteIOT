const express = require('express');
const { getDevices, registerDevice } = require('../controllers/deviceController');

const router = express.Router();

router.get('/', getDevices);
router.post('/register', registerDevice);

module.exports = router;
