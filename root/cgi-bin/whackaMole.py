#!/usr/bin/env python3
# -*- coding: utf-8 -*-

print("Content-Type: text/html; charset=utf-8")
print("")

html = """
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Whack-a-Mole Ultimate!</title>
    <style>
        :root {
            --bg-color: #121212;
            --card-color: #3a3a3a;
            --text-color: #f0f0f0;
            --score-color: #4dab7b;
            --time-color: #ff6b6b;
            --shadow-color: rgba(0,0,0,0.5);
            --hole-shadow: 0 4px 8px rgba(0,0,0,0.4);
            --difficulty-easy: #27ae60;
            --difficulty-medium: #e67e22;
            --difficulty-hard: #e74c3c;
        }
        body {
            font-family: 'Comic Sans MS', cursive, sans-serif;
            text-align: center;
            padding: 20px;
            background: var(--bg-color);
            color: var(--text-color);
        }
        h1 {
            color: #8e44ad;
            text-shadow: 2px 2px 4px var(--shadow-color);
        }
        #difficulty-selector {
            margin: 15px auto;
            display: flex;
            justify-content: center;
            gap: 10px;
        }
        .difficulty-button {
            padding: 8px 15px;
            border: none;
            border-radius: 15px;
            color: white;
            font-weight: bold;
            cursor: pointer;
            transition: transform 0.1s;
        }
        .difficulty-button:active {
            transform: scale(0.95);
        }
        #countdown {
            font-size: 3em;
            color: var(--time-color);
            margin: 20px;
            height: 60px;
            display: none;
        }
        #jeu {
            display: grid;
            grid-template-columns: repeat(3, 120px);
            gap: 15px;
            justify-content: center;
            margin: 20px auto;
            max-width: 400px;
        }
        .trou {
            width: 120px;
            height: 120px;
            background: var(--card-color);
            border-radius: 50%;
            display: flex;
            justify-content: center;
            align-items: center;
            cursor: pointer;
            position: relative;
            box-shadow: var(--hole-shadow);
            overflow: hidden;
            transition: transform 0.1s;
        }
        .trou:active {
            transform: scale(0.95);
        }
        .mole {
            width: 80px;
            height: 80px;
            background-image: url('https://pngimg.com/d/mole_PNG5.png');
            background-size: contain;
            background-repeat: no-repeat;
            background-position: center;
            transition: transform 0.1s;
            animation: popUp 0.2s ease-out;
        }
        .mole-hit {
            width: 80px;
            height: 80px;
            background-image: url('https://avatars.githubusercontent.com/u/30813356?v=4');
            background-size: contain;
            background-repeat: no-repeat;
            background-position: center;
        }
        @keyframes popUp {
            0% { transform: scale(0.5); opacity: 0; }
            80% { transform: scale(1.3); }
            100% { transform: scale(1); opacity: 1; }
        }
        #score, #highscore {
            font-size: 1.8em;
            margin: 10px;
            font-weight: bold;
        }
        #score { color: var(--score-color); }
        #highscore { color: #f1c40f; }
        #temps {
            font-size: 1.5em;
            margin: 10px;
            color: var(--time-color);
            font-weight: bold;
        }
        #feedback {
            font-size: 1.2em;
            margin: 10px;
            min-height: 24px;
            color: #e67e22;
        }
        button {
            padding: 10px 20px;
            margin: 5px;
            border: none;
            border-radius: 20px;
            background: #8e44ad;
            color: white;
            font-weight: bold;
            cursor: pointer;
            transition: background 0.2s;
        }
        button:hover {
            background: #732d91;
        }
        #retry-button {
            display: none;
        }
    </style>
</head>
<body>
    <h1>Whack-a-Mole Ultimate!</h1>
    <p id="difficulty-title">Choisis ta difficulté :</p>
    <div id="difficulty-selector">
        <button class="difficulty-button" onclick="setDifficulty('easy')" style="background: var(--difficulty-easy);">Facile</button>
        <button class="difficulty-button" onclick="setDifficulty('medium')" style="background: var(--difficulty-medium);">Moyen</button>
        <button class="difficulty-button" onclick="setDifficulty('hard')" style="background: var(--difficulty-hard);">Difficile</button>
    </div>
    <div id="countdown"></div>
    <div id="highscore">High Score: 0</div>
    <div id="score">Score: 0</div>
    <div id="temps">Temps: 30</div>
    <div id="feedback"></div>
    <div id="jeu">
        <div class="trou" data-index="0"></div>
        <div class="trou" data-index="1"></div>
        <div class="trou" data-index="2"></div>
        <div class="trou" data-index="3"></div>
        <div class="trou" data-index="4"></div>
        <div class="trou" data-index="5"></div>
    </div>
    <button id="retry-button" onclick="demanderDifficulte()">Rejouer !</button>

    <audio id="hit-sound" src="https://assets.mixkit.co/sfx/preview/mixkit-arcade-game-jump-coin-1665.mp3" preload="auto"></audio>
    <audio id="miss-sound" src="https://assets.mixkit.co/sfx/preview/mixkit-arcade-retro-changing-tab-233.mp3" preload="auto"></audio>
    <audio id="game-over-sound" src="https://assets.mixkit.co/sfx/preview/mixkit-arcade-retro-game-over-216.mp3" preload="auto"></audio>

    <script>
        // Gestion du high score (cookies)
        function getHighScore() {
            const value = `; ${document.cookie}`;
            const parts = value.split(`; highscore=`);
            if (parts.length === 2) return parseInt(parts.pop().split(';').shift()) || 0;
            return 0;
        }

        function setHighScore(score) {
            document.cookie = `highscore=${score}; path=/`;
        }

        // Variables de jeu
        let score = 0;
        let temps = 30;
        let molePositions = [];
        let jeuActif = false;
        let vitesse = 1000;
        let acceleration = 20;
        let minVitesse = 400;
        let maxMoles = 1;
        let difficulty = 'medium';
        const trous = document.querySelectorAll('.trou');
        const feedbackEl = document.getElementById('feedback');
        const highScoreEl = document.getElementById('highscore');
        const retryButton = document.getElementById('retry-button');
        const countdownEl = document.getElementById('countdown');
        let highScore = getHighScore();
        highScoreEl.textContent = `High Score: ${highScore}`;

        // Initialisation des trous
        trous.forEach(trou => {
            trou.addEventListener('click', function() {
                if (jeuActif) {
                    taper(parseInt(this.getAttribute('data-index')));
                }
            });
        });

        function setDifficulty(level) {
            difficulty = level;
            document.getElementById('difficulty-title').textContent = `Difficulté : ${level}`;
            document.querySelectorAll('.difficulty-button').forEach(btn => {
                btn.style.boxShadow = 'none';
            });
            event.target.style.boxShadow = '0 0 10px rgba(0,0,0,0.3)';
            demarrerAvecDecompte();
        }

        function demarrerAvecDecompte() {
            document.getElementById('difficulty-selector').style.display = 'none';
            document.getElementById('difficulty-title').style.display = 'none';
            countdownEl.style.display = 'block';

            let count = 3;
            countdownEl.textContent = count;
            const countdownInterval = setInterval(() => {
                count--;
                if (count > 0) {
                    countdownEl.textContent = count;
                } else {
                    clearInterval(countdownInterval);
                    countdownEl.textContent = "GO!";
                    setTimeout(() => {
                        countdownEl.style.display = 'none';
                        demarrerJeu();
                    }, 500);
                }
            }, 1000);
        }

        function afficherMoles() {
            if (!jeuActif) return;

            molePositions.forEach(pos => {
                trous[pos].innerHTML = '';
            });
            molePositions = [];

            const nbMoles = 1 + Math.floor(Math.random() * maxMoles);
            for (let i = 0; i < nbMoles; i++) {
                let pos;
                do {
                    pos = Math.floor(Math.random() * 6);
                } while (molePositions.includes(pos));
                molePositions.push(pos);
                trous[pos].innerHTML = '<div class="mole"></div>';
            }

            if (difficulty === 'easy') {
                acceleration = 10;
                minVitesse = 800;
                maxMoles = 1;
            } else if (difficulty === 'medium') {
                acceleration = 20;
                minVitesse = 600;
                maxMoles = Math.min(2, maxMoles + (temps < 15 ? 1 : 0));
            } else {
                acceleration = 30;
                minVitesse = 400;
                maxMoles = Math.min(3, maxMoles + (temps < 20 ? 1 : 0));
            }

            vitesse = Math.max(minVitesse, vitesse - acceleration);
            setTimeout(afficherMoles, vitesse);
        }

        function taper(num) {
            if (!jeuActif) return;
            if (molePositions.includes(num)) {
                score += 10;
                document.getElementById('score').innerText = 'Score: ' + score;
                trous[num].innerHTML = '<div class="mole-hit"></div>';
                document.getElementById('hit-sound').play();
                setTimeout(() => {
                    trous[num].innerHTML = '';
                }, 400);
                molePositions = molePositions.filter(pos => pos !== num);
                afficherFeedback('+10 !', true);
            } else {
                document.getElementById('miss-sound').play();
                afficherFeedback('Raté !', false);
            }
        }

        function afficherFeedback(message, isGood) {
            feedbackEl.textContent = message;
            feedbackEl.style.color = isGood ? 'var(--score-color)' : 'var(--time-color)';
            setTimeout(() => feedbackEl.textContent = '', 1000);
        }

        function decompte() {
            if (temps > 0 && jeuActif) {
                temps--;
                document.getElementById('temps').innerText = 'Temps: ' + temps;
                setTimeout(decompte, 1000);
            } else if (jeuActif) {
                jeuActif = false;
                document.getElementById('game-over-sound').play();
                if (score > highScore) {
                    highScore = score;
                    setHighScore(highScore);
                    highScoreEl.textContent = `High Score: ${highScore}`;
                }
                retryButton.style.display = 'block';
            }
        }

        function demanderDifficulte() {
            const choix = confirm(`Game Over !\nScore: ${score}\nHigh Score: ${highScore}\n\nVoulez-vous rejouer ?`);
            if (choix) {
                document.getElementById('difficulty-selector').style.display = 'flex';
                document.getElementById('difficulty-title').style.display = 'block';
                retryButton.style.display = 'none';
            }
        }

        function demarrerJeu() {
            jeuActif = true;
            score = 0;
            temps = 30;
            molePositions = [];
            vitesse = difficulty === 'easy' ? 1200 : difficulty === 'medium' ? 1000 : 800;
            maxMoles = 1;
            document.getElementById('score').innerText = 'Score: 0';
            document.getElementById('temps').innerText = 'Temps: 30';
            trous.forEach(trou => trou.innerHTML = '');
            afficherMoles();
            decompte();
        }
    </script>
</body>
</html>
"""
print(html)