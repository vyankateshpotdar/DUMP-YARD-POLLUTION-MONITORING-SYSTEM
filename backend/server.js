const express = require('express');
const mongoose = require('mongoose');
const nodemailer = require('nodemailer');
require('dotenv').config();

const app = express();
app.use(express.json());

const pollutionSchema = new mongoose.Schema({
  temperature: Number,
  humidity: Number,
  timestamp: { type: Date, default: Date.now },
  severity: String
});

const Pollution = mongoose.model('Pollution', pollutionSchema);

mongoose.connect(process.env.MONGODB_URI || 'mongodb://localhost:27017/pollution_monitor', {
  useNewUrlParser: true,
  useUnifiedTopology: true
});

const transporter = nodemailer.createTransport({
  service: 'gmail',
  auth: {
    user: process.env.EMAIL_USER,
    pass: process.env.EMAIL_PASSWORD
  }
});

app.post('/api/pollution/log', async (req, res) => {
  const { temperature, humidity } = req.body;
  let severity = 'LOW';
  if (temperature > 35 && humidity > 75) severity = 'HIGH';
  
  const record = new Pollution({ temperature, humidity, severity });
  await record.save();
  
  if (severity === 'HIGH') {
    await transporter.sendMail({
      from: process.env.EMAIL_USER,
      to: process.env.WARDEN_EMAIL,
      subject: '🔴 POLLUTION ALERT',
      html: `<h2>Alert!</h2><p>Temp: ${temperature}°C | Humidity: ${humidity}%</p>`
    });
  }
  
  res.json({ success: true, severity });
});

app.get('/api/pollution/latest', async (req, res) => {
  const latest = await Pollution.findOne().sort({ timestamp: -1 });
  res.json(latest);
});

app.get('/api/pollution/stats', async (req, res) => {
  const stats = await Pollution.aggregate([
    { $group: { _id: null, avgTemp: { $avg: '$temperature' }, avgHumidity: { $avg: '$humidity' }, maxTemp: { $max: '$temperature' }, maxHumidity: { $max: '$humidity' }, count: { $sum: 1 } } }
  ]);
  res.json(stats[0]);
});

app.get('/api/pollution/alerts', async (req, res) => {
  const alerts = await Pollution.find({ severity: 'HIGH' }).sort({ timestamp: -1 }).limit(50);
  res.json(alerts);
});

app.listen(3000, () => console.log('✅ Server on port 3000'));
