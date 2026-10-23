import express from "express";

const app = express();

app.get('/',(req,res)=>{
    res.send("Hello from the backend server on port 8081");
});

app.listen(8081,()=>{
    console.log("Backend server running on http://localhost:8081");
});
