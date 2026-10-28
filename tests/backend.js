import express from "express";

const app = express();

const port = process.argv[2] || 8081;

app.get('/',(req,res)=>{
    res.send("Hello from the backend server on port"+port);
});

app.listen(port,()=>{
    console.log("Backend server running on http://localhost:"+port);
});
