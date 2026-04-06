import os
import random

fortunes = [
    "You will fix your segfault... eventually.",
    "A wild memory leak approaches.",
    "You will forget a semicolon today.",
    "rm -rf / is not the solution.",
    "Have you tried turning it off and on again?",
    "Your code works. You don't know why. Don't touch it.",
    "A git push --force is in your future.",
    "The bug is between the chair and the keyboard.",
    "Undefined behavior will find you.",
    "The segfault is coming from inside the house.",
    "You will write perfect code today. (This is a lie.)",
    "You will give this project 125/125",
	"You will give this project 125/125 AND OUTSTANDING ;))))))",
    "Today is a good day.",
    "The future is cloudy, take a jacket",
    "Be wary of cats, they bite sometimes",
	"Be wary of cats, they bite sometimes",
    "Be wary of cats, they bite sometimes",
    "Be wary of cats, they bite sometimes",
    "Be wary of cats, they bite sometimes",
    "Be wary of cats, they bite sometimes",
    "Be wary of cats, they bite sometimes",
    "Your next commit will break the build.",
    "You will find a stack overflow answer that actually works.",
    "You will inherit a legacy codebase. May the odds be ever in your favor.",
	"                            __\n"
    " ,                    ,\" e`--o\n"
    "((                   (  | __,'\n"
    " \\~----------------' \_;/\n"
	"  (                      /\n"
    " /) ._______________.  )\n"
    "(( (               (( (\n"
    "``-'               ``-'",
	"You will give this project 125/125",
	"You will give this project 125/125",
	"You will give this project 125/125",
	"You will give this project 125/125",
	"You will give this project 125/125",
	"You will give this project 125/125",
	"You will give this project 125/125",
	"You will give this project 125/125",
	"You will give this project 125/125",
    "Your pull request will be merged without a single comment.",
    "Your terminal will display a rainbow today.",
    "You will learn something new and unexpected.",
    "Your coffee will be cold before you take the first sip."
]

name = os.environ.get("QUERY_STRING", "")
if name.startswith("name="):
    name = name[5:].replace("+", " ")
else:
    name = "stranger"

fortune = random.choice(fortunes)

print("Content-Type: text/html")
print()
print(f"""<!DOCTYPE html>
<html><head><meta charset='UTF-8'><title>Fortune Teller</title>
<style>
body {{ font-family: Arial, sans-serif; background: #1e1e1e; color: #f0f0f0;
       display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }}
.card {{ background: #2a2a2a; border-radius: 16px; padding: 40px 60px; text-align: center;
         box-shadow: 0 0 30px rgba(100, 200, 255, 0.2); max-width: 600px; }}
h1 {{ color: #6cf; }}
.fortune {{ font-size: 1.4em; margin: 20px 0; font-style: italic; color: #ffd700; }}
button {{
    background: #6cf; color: white; border: none; padding: 10px 20px; border-radius: 8px;
    font-size: 1em; cursor: pointer; margin-top: 20px;
    transition: background 0.3s;
}}
button:hover {{ background: #5bc0eb; }}
</style></head>
<body><div class='card'>
<div style='font-size: 4em'>&#x1F52E;</div>
<h1>Fortune Teller</h1>
<p>Ah, {name}... the crystal ball speaks:</p>
<div class='fortune' id='fortune'>&#x201C;{fortune}&#x201D;</div>
<button onclick="window.location.reload()">Tell me another!</button>
<p style='color: #aaa; font-size: 0.9em'>Or just keep clicking for more wisdom</p>
</div></body></html>""")