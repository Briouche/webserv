<?php
$name = isset($_GET['name']) ? $_GET['name'] : "BigMan";
$description = isset($_GET['description']) ? $_GET['description'] : "IMPRESSIVE";

echo "<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <title>Hello!</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #1e1e1e;
            color: #f0f0f0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }
        .card {
            background: #2a2a2a;
            border-radius: 16px;
            padding: 40px 60px;
            text-align: center;
            box-shadow: 0 0 30px rgba(100, 200, 255, 0.2);
        }
        h1 { color: #6cf; font-size: 2.5em; }
        p  { font-size: 1.2em; margin: 10px 0; }
    </style>
</head>
<body>
    <div class='card'>
        <div style='font-size: 4em'>😄</div>
        <h1>Hello, $name! 👋</h1>
        <p>🎂 You are <strong>$description</strong> !</p>
        <p>😎 Welcome to our webserv!</p>
        <p>🚀 Hope everything is working well!</p>
    </div>
</body>
</html>";
?>