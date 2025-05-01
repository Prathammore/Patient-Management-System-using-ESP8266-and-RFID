const express = require("express");
const mongoose = require("mongoose");
const bodyParser = require("body-parser");
const path = require("path");
const uidStore = require("./uid");
const { encryptUID } = require("./rsa");

const app = express();
const PORT = 3000;

mongoose.connect("mongodb://127.0.0.1:27017/patientDB");

const db = mongoose.connection;
db.once("open", () => console.log("✅ MongoDB Connected"));

const patientSchema = new mongoose.Schema({
  uid: String,
  encryptUID: String,
  name: String,
  age: Number,
  gender: String,
  disease: String,
  entryAt: { type: Date, default: Date.now },
  createdAt: { type: Date, default: Date.now }
});

const Patient = mongoose.model("Patient", patientSchema);

app.use(bodyParser.urlencoded({ extended: true }));
app.use(express.static(path.join(__dirname, "../public")));

app.get("/check", async (req, res) => {
  try {
    const { uid } = req.query;

    if (!uid) {
      return res.status(400).send("UID is required");
    }

    uidStore.setUID(uid);

    const patient = await Patient.findOne({ uid });

    res.status(200).send(patient ? "existing" : "new");
  } catch (error) {
    console.error("Error checking patient UID:", error);
    res.status(500).send("Server error");
  }
});

app.delete("/delete", async (req, res) => {
  const { uid } = req.query;

  if (!uid) return res.status(400).send("UID is required");

  const deleted = await Patient.deleteOne({ uid });

  if (deleted.deletedCount === 0) return res.status(404).send("Patient not found");
  res.send("Patient deleted successfully");
});

app.get("/search", async (req, res) => {
  const { query } = req.query;
  try {
    if (!query) return res.status(400).json({ error: "Search query is required" });

    
    const patients = await Patient.find({
      name: { $regex: query, $options: "i" }
    });

    res.json(patients);
  } catch (error) {
    console.error("Search error:", error);
    res.status(500).json({ error: "Internal server error" });
  }
});

app.get("/get-uid", async (req, res) => {
  try {
    const uid = uidStore.getUID();

    if (!uid) return res.json({ status: "waiting" });

    const patient = await Patient.findOne({ uid });

    uidStore.clearUID();

    res.json({
      status: patient ? "existing" : "new",
      uid: uid,
    });
  } catch (error) {
    console.error("Error polling UID:", error);
    res.status(500).send("Server error");
  }
});

app.post("/register", async (req, res) => {
  const { uid, name, age, gender, disease } = req.body;

  const exists = await Patient.findOne({ uid });
  if (exists) {
  
    exists.name = name;
    exists.age = age;
    exists.gender = gender;
    exists.disease = disease;
    await exists.save();
    res.redirect(`/details.html?uid=${uid}`);
  } else {
        const newPatient = new Patient({
      uid,
      name,
      age,
      gender,
      disease,
    });
    await newPatient.save();
    res.redirect(`/details.html?uid=${uid}`);
  }
});

app.get("/details", async (req, res) => {
  const { uid } = req.query;
  const patient = await Patient.findOne({ uid });

  if (!patient) return res.json({ error: "Patient not found." });

  res.json({
    uid: patient.uid,
    name: patient.name,
    age: patient.age,
    gender: patient.gender,
    disease: patient.disease,
    createdAt: patient.createdAt,
    entryAt: patient.entryAt,
  });
});


app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "../public/home.html"));
});


app.listen(PORT, () => {
  console.log(`Server running at http://localhost:${PORT}`);
});
