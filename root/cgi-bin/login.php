<?php
$valid_user = "admin";
$valid_pass = "42";

$username = $_POST['username'] ?? '';
$password = $_POST['password'] ?? '';

if ($username === $valid_user && $password === $valid_pass) {
    $session_id = bin2hex(random_bytes(16));

    setcookie("session_id", $session_id, [
        'path' => '/',
        'httponly' => true,
        'samesite' => 'Strict'
    ]);

    header("Location: /welcome.html");
    http_response_code(302);
    echo "<html><body>Redirecting...</body></html>";
} else {
    echo "<html><body>Login failed. <a href='/login.html'>Try again</a></body></html>";
}
?>