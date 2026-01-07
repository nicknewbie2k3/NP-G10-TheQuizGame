// WebSocket connection
let ws = null;
let isHost = false;
let currentGamePin = null;
let playerId = null;
let playerName = null;
let currentQuestion = null;
let players = [];
let playerEliminated = false;

// Connect to WebSocket server
function connectWebSocket() {
    ws = new WebSocket('ws://localhost:8080');
    
    ws.onopen = () => {
        console.log('✅ Connected to game server');
    };
    
    ws.onmessage = (event) => {
        try {
            const message = JSON.parse(event.data);
            console.log('📨 Received:', message);
            handleMessage(message);
        } catch (error) {
            console.error('❌ Error parsing message:', error);
        }
    };
    
    ws.onerror = (error) => {
        console.error('❌ WebSocket error:', error);
        showError('Connection error. Please try again.');
    };
    
    ws.onclose = () => {
        console.log('🔌 Disconnected from server');
        showError('Disconnected from server');
    };
}

// Send message to server
function sendMessage(message) {
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify(message));
    } else {
        console.error('❌ WebSocket not connected');
    }
}

// Message handler
function handleMessage(message) {
    switch (message.type) {
        case 'game_created':
            handleGameCreated(message);
            break;
        case 'join_success':
            handleJoinSuccess(message);
            break;
        case 'player_joined':
            handlePlayerJoined(message);
            break;
        case 'game_started':
            handleGameStarted(message);
            break;
        case 'new_question':
            handleNewQuestion(message);
            break;
        case 'answer_received':
            handleAnswerReceived(message);
            break;
        case 'question_results':
            handleQuestionResults(message);
            break;
        case 'round_complete':
            handleRoundComplete(message);
            break;
        case 'player_eliminated':
            handlePlayerEliminated(message);
            break;
        case 'round2_start':
            handleRound2Start(message);
            break;
        case 'speed_question':
            handleSpeedQuestion(message);
            break;
        case 'speed_answer_received':
            handleSpeedAnswerReceived(message);
            break;
        case 'speed_results':
            handleSpeedResults(message);
            break;
        case 'tiebreak_start':
            handleTiebreakStart(message);
            break;
        case 'tiebreak_question':
            handleTiebreakQuestion(message);
            break;
        case 'tiebreak_answer_received':
            handleTiebreakAnswerReceived(message);
            break;
        case 'tiebreak_results':
            handleTiebreakResults(message);
            break;
        case 'player_order':
            handlePlayerOrder(message);
            break;
        case 'round2_player_order':
            handleRound2PlayerOrder(message);
            break;
        case 'round2_packs_available':
            handleRound2PacksAvailable(message);
            break;
        case 'pack_selected':
            handlePackSelected(message);
            break;
        case 'pack_waiting_host':
            handlePackWaitingHost(message);
            break;
        case 'pack_questions':
            handlePackQuestions(message);
            break;
        case 'pack_answer_verified':
            handlePackAnswerVerified(message);
            break;
        case 'pack_complete':
            handlePackComplete(message);
            break;
        case 'turn_ended':
            handleTurnEnded(message);
            break;
        case 'round2_complete':
            handleRound2Complete(message);
            break;
        case 'game_ended':
            handleGameEnded(message);
            break;
        case 'error':
            showError(message.message);
            break;
        default:
            console.log('Unknown message type:', message.type);
    }
}

// Show host game
function showHostGame() {
    if (!ws || ws.readyState !== WebSocket.OPEN) {
        connectWebSocket();
        setTimeout(() => {
            sendMessage({ type: 'create_game' });
        }, 500);
    } else {
        sendMessage({ type: 'create_game' });
    }
    isHost = true;
}

// Show join game
function showJoinGame() {
    showScreen('join-game');
}

// Join game
function joinGame(event) {
    event.preventDefault();
    
    const pin = document.getElementById('game-pin').value.toUpperCase();
    const name = document.getElementById('player-name').value;
    
    if (!pin || !name) {
        showError('Please enter both PIN and name');
        return;
    }
    
    currentGamePin = pin;
    playerName = name;
    
    if (!ws || ws.readyState !== WebSocket.OPEN) {
        connectWebSocket();
        setTimeout(() => {
            sendMessage({
                type: 'join_game',
                gamePin: pin,
                playerName: name
            });
        }, 500);
    } else {
        sendMessage({
            type: 'join_game',
            gamePin: pin,
            playerName: name
        });
    }
}

// Start game (host only)
function startGame() {
    sendMessage({ type: 'start_game' });
}

// Submit answer
function submitAnswer(questionId, answer) {
    sendMessage({
        type: 'submit_answer',
        questionId: questionId,
        answer: answer
    });
    
    // Disable all options
    const options = document.querySelectorAll('.option-btn');
    options.forEach(btn => btn.disabled = true);
}

// Submit speed answer
function submitSpeedAnswer(event) {
    event.preventDefault();
    
    // Prevent eliminated players from submitting answers
    if (playerEliminated) {
        showError('You have been eliminated and cannot answer');
        return;
    }
    
    const answer = document.getElementById('speed-answer-input').value.trim();
    
    if (!answer) {
        showError('Please enter an answer');
        return;
    }
    
    sendMessage({
        type: 'submit_speed_answer',
        questionId: currentQuestion.id,
        answer: answer
    });
    
    console.log('Speed answer submitted:', answer);
}

function submitTiebreakAnswer(event) {
    event.preventDefault();
    
    const answer = document.getElementById('tiebreak-answer-input').value.trim();
    
    if (!answer) {
        showError('Please enter an answer');
        return;
    }
    
    sendMessage({
        type: 'submit_tiebreak_answer',
        answer: answer
    });
    
    console.log('Tiebreak answer submitted:', answer);
}

// Next question (host only)
function nextQuestion() {
    sendMessage({ type: 'next_question' });
}

// Next round (host only)
function nextRound() {
    sendMessage({ type: 'next_round' });
}

// End game (host only)
function endGame() {
    if (confirm('Are you sure you want to end the game?')) {
        sendMessage({ type: 'end_game' });
        showLanding();
    }
}

// Handle game created
function handleGameCreated(message) {
    currentGamePin = message.gamePin;
    document.getElementById('host-pin').textContent = message.gamePin;
    showScreen('host-lobby');
}

// Handle join success
function handleJoinSuccess(message) {
    playerId = message.playerId;
    currentGamePin = message.gamePin;
    playerName = message.playerName;
    showScreen('player-lobby');
}

// Handle player joined
function handlePlayerJoined(message) {
    players = message.players;
    updatePlayerList();
}

// Handle game started
function handleGameStarted(message) {
    if (isHost) {
        showScreen('question-screen');
    } else {
        showScreen('question-screen');
    }
}

// Handle new question
function handleNewQuestion(message) {
    currentQuestion = message.question;
    
    document.getElementById('current-round').textContent = message.round;
    document.getElementById('question-number').textContent = message.questionNumber;
    document.getElementById('total-questions').textContent = message.totalQuestions;
    document.getElementById('question-text').textContent = message.question.text;
    
    // Display options
    const optionsGrid = document.getElementById('options-grid');
    optionsGrid.innerHTML = '';
    
    message.question.options.forEach((option, index) => {
        const btn = document.createElement('button');
        btn.className = 'option-btn';
        btn.textContent = option;
        btn.onclick = () => submitAnswer(message.question.id, index);
        optionsGrid.appendChild(btn);
    });
    
    // Start timer
    startTimer(message.question.timeLimit);
    
    // Hide feedback
    document.getElementById('answer-feedback').classList.add('hidden');
    
    showScreen('question-screen');
}

// Handle answer received
function handleAnswerReceived(message) {
    const feedback = document.getElementById('answer-feedback');
    feedback.className = 'answer-feedback';
    
    if (message.correct) {
        feedback.classList.add('correct');
        feedback.textContent = '✅ Correct!';
    } else {
        feedback.classList.add('incorrect');
        feedback.textContent = '❌ Incorrect';
    }
    
    feedback.classList.remove('hidden');
}

// Handle question results
function handleQuestionResults(message) {
    const resultsContent = document.getElementById('results-content');
    resultsContent.innerHTML = '';
    
    const correctAnswer = currentQuestion.options[message.correctAnswer];
    
    const answerDiv = document.createElement('div');
    answerDiv.className = 'correct-answer-display';
    answerDiv.innerHTML = `<strong>Correct Answer:</strong> ${correctAnswer}`;
    resultsContent.appendChild(answerDiv);
    
    const scoresDiv = document.createElement('div');
    scoresDiv.className = 'scores-display';
    scoresDiv.innerHTML = '<h3>Scores:</h3>';
    
    const scoresList = document.createElement('ul');
    scoresList.className = 'scores-list';
    
    Object.entries(message.scores).forEach(([pid, score]) => {
        const player = players.find(p => p.id === pid);
        if (player) {
            const li = document.createElement('li');
            li.textContent = `${player.name}: ${score} points`;
            scoresList.appendChild(li);
        }
    });
    
    scoresDiv.appendChild(scoresList);
    resultsContent.appendChild(scoresDiv);
    
    if (isHost) {
        document.getElementById('next-question-btn').style.display = 'block';
    } else {
        document.getElementById('next-question-btn').style.display = 'none';
    }
    
    showScreen('results-screen');
}

// Handle round complete
function handleRoundComplete(message) {
    showScreen('round-complete');
}

// Handle player eliminated
function handlePlayerEliminated(message) {
    if (message.playerId === playerId) {
        playerEliminated = true;
        showScreen('eliminated-screen');
    } else {
        showNotification(`${message.playerName} has been eliminated!`);
    }
}

// Handle Round 2 start
function handleRound2Start(message) {
    console.log('Round 2 starting - Speed Question Phase!');
    // Eliminated players should stay on eliminated screen
    if (playerEliminated) {
        console.log('Player is eliminated, staying on eliminated screen');
        return;
    }
    // Wait for speed_question message for active players
}

// Handle speed question
function handleSpeedQuestion(message) {
    // Don't show speed question to eliminated players
    if (playerEliminated) {
        console.log('Player is eliminated, not showing speed question');
        return;
    }
    
    const question = message.question;
    currentQuestion = question;
    
    document.getElementById('speed-question-text').textContent = question.text;
    document.getElementById('speed-answer-input').value = '';
    document.getElementById('speed-answer-input').focus();
    document.getElementById('speed-waiting').style.display = 'none';
    
    showScreen('speed-question-screen');
}

// Handle speed answer received
function handleSpeedAnswerReceived(message) {
    document.getElementById('speed-waiting').style.display = 'block';
    document.getElementById('speed-answer-form').style.display = 'none';
}

// Handle speed results
function handleSpeedResults(message) {
    console.log('Received speed results:', message);
    
    const results = message.results;
    if (!results || !Array.isArray(results)) {
        console.error('Invalid results format:', message);
        return;
    }
    
    const content = document.getElementById('speed-results-content');
    
    let html = '<div class="leaderboard">';
    html += '<h3>⚡ Speed Question Results</h3>';
    html += '<div class="speed-results-header">';
    html += '<p>Players ranked by answer speed (fastest first)</p>';
    html += '</div>';
    html += '<ol class="speed-results-list">';
    
    results.forEach((result, index) => {
        const medal = index === 0 ? '🥇' : index === 1 ? '🥈' : index === 2 ? '🥉' : `#${index + 1}`;
        const correct = result.correct ? '✅' : '❌';
        html += `
            <li class="speed-result-item ${result.correct ? 'correct' : 'incorrect'}">
                <div class="result-position">
                    <span class="medal">${medal}</span>
                </div>
                <div class="result-player">
                    <span class="player-name">${result.playerName}</span>
                </div>
                <div class="result-answer">
                    <span class="answer-label">Answer:</span>
                    <span class="answer-value">${result.answer}</span>
                </div>
                <div class="result-status">
                    <span class="status-icon">${correct}</span>
                    <span class="status-text">${result.correct ? 'Correct' : 'Incorrect'}</span>
                </div>
                <div class="result-time">
                    <span class="time-icon">⏱️</span>
                    <span class="time-value">${result.responseTime.toFixed(2)}s</span>
                </div>
            </li>
        `;
    });
    
    html += '</ol></div>';
    
    if (message.eliminated) {
        html += `<div class="elimination-notice">
            <p>⚠️ <strong>${message.eliminated.playerName}</strong> was eliminated</p>
        </div>`;
    }
    
    content.innerHTML = html;
    
    // Show host controls if host
    if (isHost) {
        document.getElementById('speed-host-controls').style.display = 'block';
        document.getElementById('speed-waiting').style.display = 'none';
    } else {
        document.getElementById('speed-host-controls').style.display = 'none';
        document.getElementById('speed-waiting').style.display = 'block';
    }
    
    showScreen('speed-results-screen');
}

// Handle tiebreak start
function handleTiebreakStart(message) {
    const content = document.getElementById('tiebreak-notice-content');
    html = `<div class="tiebreak-alert">
        <h3>⚡ Tiebreaker Detected!</h3>
        <p>${message.tiedPlayerCount} players tied with the same score.</p>
        <p>A speed question will determine who advances...</p>
        <div class="spinner"></div>
    </div>`;
    content.innerHTML = html;
    showScreen('tiebreak-notice-screen');
}

// Handle tiebreak question
function handleTiebreakQuestion(message) {
    const question = message.question;
    currentQuestion = question;
    
    document.getElementById('tiebreak-question-text').textContent = question.text;
    document.getElementById('tiebreak-answer-input').value = '';
    document.getElementById('tiebreak-answer-input').focus();
    document.getElementById('tiebreak-waiting').style.display = 'none';
    
    showScreen('tiebreak-question-screen');
}

// Handle tiebreak answer received
function handleTiebreakAnswerReceived(message) {
    document.getElementById('tiebreak-waiting').style.display = 'block';
    document.getElementById('tiebreak-answer-form').style.display = 'none';
}

// Handle tiebreak results
function handleTiebreakResults(message) {
    const results = message.results;
    const content = document.getElementById('tiebreak-results-content');
    
    let html = '<div class="leaderboard">';
    html += '<h3>Tiebreaker Results (Fastest First):</h3>';
    html += '<ol class="tiebreak-results-list">';
    
    results.forEach((result, index) => {
        const medal = index === 0 ? '🥇' : index === 1 ? '🥈' : index === 2 ? '🥉' : '';
        const correct = result.correct ? '✅' : '❌';
        html += `
            <li class="tiebreak-result-item ${result.correct ? 'correct' : 'incorrect'}">
                <span class="medal">${medal}</span>
                <span class="player-name">${result.playerName}</span>
                <span class="answer">${result.answer}</span>
                <span class="status">${correct}</span>
                <span class="time">${result.responseTime.toFixed(2)}s</span>
            </li>
        `;
    });
    
    html += '</ol></div>';
    
    if (message.eliminated) {
        html += `<div class="elimination-notice">
            <p>⚠️ <strong>${message.eliminated.playerName}</strong> has been eliminated!</p>
        </div>`;
        
        // Mark current player as eliminated if they lost the tiebreaker
        if (message.eliminated.playerId === playerId) {
            playerEliminated = true;
        }
    }
    
    content.innerHTML = html;
    
    // Show host controls if host, otherwise show waiting
    if (isHost) {
        document.getElementById('tiebreak-host-controls').style.display = 'block';
        document.getElementById('tiebreak-waiting').style.display = 'none';
    } else {
        document.getElementById('tiebreak-host-controls').style.display = 'none';
        document.getElementById('tiebreak-waiting').style.display = 'block';
    }
    
    showScreen('tiebreak-results-screen');
}

function continueToRound2() {
    sendMessage({
        type: 'continue_from_speed_order'
    });
    console.log('Host continuing from speed order to Round 2...');
}

// Handle player order announcement
function handlePlayerOrder(message) {
    const content = document.getElementById('player-order-content');
    
    let html = '<div class="order-announcement">';
    html += '<h3>📋 ' + message.message + '</h3>';
    html += '<ol class="player-order-list">';
    
    message.order.forEach((player) => {
        const medal = player.position === 1 ? '🥇' : player.position === 2 ? '🥈' : '🥉';
        html += `
            <li class="order-item">
                <span class="medal">${medal}</span>
                <span class="position">#${player.position}</span>
                <span class="player-name">${player.playerName}</span>
                <span class="time">${player.responseTime.toFixed(2)}s</span>
            </li>
        `;
    });
    
    html += '</ol>';
    html += '<p class="order-subtitle">This is your turn order for Round 2</p>';
    html += '</div>';
    
    content.innerHTML = html;
    showScreen('player-order-screen');
}

// Handle Round 2 questions start
function handleRound2QuestionsStart(message) {
    const content = document.getElementById('round2-actual-start');
    html = `<div class="round-transition">
        <h2>📚 ${message.message}</h2>
        <p>Loading question packs...</p>
        <div class="spinner"></div>
    </div>`;
    content.innerHTML = html;
    showScreen('round2-questions-start-screen');
}

// Global variable to track turn-based play state
let round2PlayerOrder = [];
let round2CurrentTurnIndex = 0;
let round2SelectedPacks = new Set();

// Handle player order for Round 2
function handleRound2PlayerOrder(message) {
    console.log('Received Round 2 player order:', message);
    round2PlayerOrder = message.playerOrder || [];
    round2CurrentTurnIndex = 0;
}

// Handle question packs available
function handleRound2PacksAvailable(message) {
    console.log('Received Round 2 question packs:', message);
    
    const content = document.getElementById('round2-packs-content');
    
    // Update selected packs from server data
    (message.packs || []).forEach((pack) => {
        if (pack.selected) {
            round2SelectedPacks.add(pack.id);
        }
    });
    
    // Move to next available player turn
    if (round2CurrentTurnIndex > 0) {
        round2CurrentTurnIndex = (round2CurrentTurnIndex + 1) % round2PlayerOrder.length;
    }
    
    // Show current player's turn
    const currentPlayer = round2PlayerOrder[round2CurrentTurnIndex];
    let html = '<div class="round2-header">';
    html += `<h2>📚 Round 2: Question Packs - Turn-Based Play</h2>`;
    html += `<div class="turn-indicator">`;
    if (currentPlayer) {
        const isYourTurn = currentPlayer.playerId === playerId;
        html += `<p class="turn-text">🎯 ${isYourTurn ? 'YOUR TURN' : "It's " + currentPlayer.playerName + "'s turn"}</p>`;
    }
    html += `</div>`;
    html += '</div>';
    
    // Show player turn order
    html += '<div class="player-turn-order">';
    html += '<h3>Turn Order:</h3>';
    html += '<ol class="turn-order-list">';
    round2PlayerOrder.forEach((p, idx) => {
        const isCurrent = idx === round2CurrentTurnIndex;
        html += `<li class="turn-order-item ${isCurrent ? 'current' : ''}">`;
        if (isCurrent) html += '👉 ';
        html += `<span class="turn-number">#${idx + 1}</span> ${p.playerName}`;
        html += `</li>`;
    });
    html += '</ol>';
    html += '</div>';
    
    // Show question packs
    html += '<div class="packs-grid">';
    html += '<h3>Available Question Packs:</h3>';
    html += '<div class="packs-container">';
    
    (message.packs || []).forEach((pack) => {
        const isSelected = pack.selected || round2SelectedPacks.has(pack.id);
        const isSelectable = currentPlayer && currentPlayer.playerId === playerId && !isSelected;
        html += `
            <div class="pack-card ${isSelected ? 'selected' : ''} ${isSelectable ? 'selectable' : ''}" data-pack-id="${pack.id}" data-selectable="${isSelectable}">
                <div class="pack-title">${pack.title}</div>
                <div class="pack-description">${pack.description}</div>
                <div class="pack-questions">📝 ${pack.questionCount} questions</div>
                ${isSelectable ? 
                    `<button class="btn btn-secondary pack-select-btn" data-pack-id="${pack.id}">Select</button>` : 
                    ''}
                ${isSelected ? '<div class="selected-badge">✅ Selected</div>' : ''}
            </div>
        `;
    });
    
    html += '</div>';
    html += '</div>';
    
    content.innerHTML = html;
    showScreen('round2-turnbased-screen');
    
    // Add event listeners after HTML is inserted
    const packCards = document.querySelectorAll('.pack-card[data-selectable="true"]');
    packCards.forEach(card => {
        card.addEventListener('click', function() {
            const packId = this.getAttribute('data-pack-id');
            selectQuestionPack(packId);
        });
    });
    
    const selectButtons = document.querySelectorAll('.pack-select-btn');
    selectButtons.forEach(btn => {
        btn.addEventListener('click', function(e) {
            e.stopPropagation();
            const packId = this.getAttribute('data-pack-id');
            selectQuestionPack(packId);
        });
    });
}

// Handle question pack selection
function selectQuestionPack(packId) {
    console.log('selectQuestionPack called with:', packId);
    
    if (!ws || ws.readyState !== WebSocket.OPEN) {
        console.error('WebSocket not connected!');
        showError('Not connected to server. Please refresh the page.');
        return;
    }
    
    const message = {
        type: 'select_question_pack',
        packId: packId
    };
    
    console.log('Sending message:', message);
    sendMessage(message);
    console.log('Selected question pack:', packId);
}

// Handle pack selected notification
function handlePackSelected(message) {
    console.log('Pack selected:', message);
    // Just log it - the pack_waiting_host message will display the screen
}

// Handle waiting for host to start pack questions
function handlePackWaitingHost(message) {
    console.log('Waiting for host to start pack:', message);
    
    const content = document.getElementById('pack-waiting-content');
    
    let html = '<div class="pack-waiting-container">';
    html += `<h2>📦 ${message.packTitle}</h2>`;
    html += `<p class="pack-selected-by">Selected by: <strong>${message.playerName}</strong></p>`;
    
    if (isHost) {
        html += '<div class="host-start-section">';
        html += '<p class="host-instruction">Ready to start the questions?</p>';
        html += '<button class="btn btn-primary btn-large start-pack-btn">▶️ Start Questions</button>';
        html += '</div>';
    } else {
        html += '<div class="waiting-message">';
        html += '<p>Waiting for host to start the questions...</p>';
        html += '<div class="spinner"></div>';
        html += '</div>';
    }
    
    html += '</div>';
    
    content.innerHTML = html;
    showScreen('pack-waiting-screen');
    
    // Add event listener for start button
    if (isHost) {
        const startBtn = document.querySelector('.start-pack-btn');
        if (startBtn) {
            startBtn.addEventListener('click', startPackQuestions);
        }
    }
}

// Host starts pack questions
function startPackQuestions() {
    sendMessage({
        type: 'start_pack_questions'
    });
    console.log('Host starting pack questions...');
}

// Global timer variable
let packTimer = null;
let packTimeRemaining = 0;
let currentPackQuestions = [];
let currentPackQuestionIndex = 0;
let currentPackScore = 0;
let currentPackPlayer = '';

// Handle pack questions display
function handlePackQuestions(message) {
    console.log('Received pack questions:', message);
    
    // Clear any existing timer
    if (packTimer) {
        clearInterval(packTimer);
    }
    
    packTimeRemaining = message.timeLimit || 45;
    currentPackQuestions = message.questions || [];
    currentPackQuestionIndex = 0;
    currentPackScore = 0;
    currentPackPlayer = message.currentPlayer || 'Player';
    
    showScreen('pack-questions-screen');
    displayCurrentPackQuestion();
    
    // Start timer
    startPackTimer();
}

// Display the current question in the pack
function displayCurrentPackQuestion() {
    const content = document.getElementById('pack-questions-content');
    
    if (currentPackQuestionIndex >= currentPackQuestions.length) {
        // All questions answered
        let html = '<div class="pack-complete-container">';
        html += `<h2>✅ Pack Complete!</h2>`;
        html += `<p class="final-score">Final Score: ${currentPackScore}/${currentPackQuestions.length}</p>`;
        html += '</div>';
        content.innerHTML = html;
        
        if (packTimer) {
            clearInterval(packTimer);
        }
        return;
    }
    
    const question = currentPackQuestions[currentPackQuestionIndex];
    
    let html = '<div class="pack-questions-container">';
    html += `<div class="pack-header">`;
    html += `<h2>📚 Question Pack</h2>`;
    html += `<p class="current-player-turn">${currentPackPlayer}'s Turn</p>`;
    html += `</div>`;
    
    // Score and timer row
    html += '<div class="pack-status-row">';
    html += `<div class="pack-score">`;
    html += `<div class="score-label">Score:</div>`;
    html += `<div class="score-value">${currentPackScore}</div>`;
    html += `</div>`;
    html += `<div class="pack-timer" id="pack-timer">`;
    html += `<div class="timer-label">Time:</div>`;
    html += `<div class="timer-value" id="timer-value">${packTimeRemaining}s</div>`;
    html += `</div>`;
    html += '</div>';
    
    // Question progress
    html += `<div class="question-progress">`;
    html += `Question ${currentPackQuestionIndex + 1} of ${currentPackQuestions.length}`;
    html += `</div>`;
    
    // Current question
    html += '<div class="current-question-display">';
    html += `<div class="question-text-large">${question.text}</div>`;
    html += `<div class="question-answer-reveal">`;
    html += `<strong>Answer:</strong> <span class="answer-highlight">${question.answer}</span>`;
    html += `</div>`;
    html += '</div>';
    
    // Host controls
    if (isHost) {
        html += '<div class="host-verification-buttons">';
        html += '<button class="btn btn-success btn-large verify-btn" data-correct="true">✅ Correct (+1)</button>';
        html += '<button class="btn btn-danger btn-large verify-btn" data-correct="false">❌ Incorrect (+0)</button>';
        html += '</div>';
    } else {
        html += '<div class="player-waiting">';
        html += '<p>Waiting for host verification...</p>';
        html += '</div>';
    }
    
    html += '</div>';
    
    content.innerHTML = html;
    
    // Add event listeners for host buttons
    if (isHost) {
        const verifyButtons = document.querySelectorAll('.verify-btn');
        verifyButtons.forEach(btn => {
            btn.addEventListener('click', function() {
                const isCorrect = this.getAttribute('data-correct') === 'true';
                verifyAnswer(isCorrect);
            });
        });
    }
}

// Host verifies answer
function verifyAnswer(isCorrect) {
    console.log('Verifying answer:', isCorrect);
    
    // Update score
    if (isCorrect) {
        currentPackScore++;
    }
    
    // Send to server
    sendMessage({
        type: 'pack_answer_verified',
        isCorrect: isCorrect,
        questionIndex: currentPackQuestionIndex
    });
    
    // Move to next question
    currentPackQuestionIndex++;
    
    // Small delay before showing next question
    setTimeout(() => {
        displayCurrentPackQuestion();
    }, 500);
}

// Start the pack timer
function startPackTimer() {
    packTimer = setInterval(() => {
        packTimeRemaining--;
        
        const timerElement = document.getElementById('timer-value');
        if (timerElement) {
            timerElement.textContent = packTimeRemaining + 's';
            
            // Change color as time runs out
            if (packTimeRemaining <= 10) {
                timerElement.style.color = '#dc3545';
                timerElement.classList.add('timer-warning');
            } else if (packTimeRemaining <= 20) {
                timerElement.style.color = '#ffc107';
            }
        }
        
        if (packTimeRemaining <= 0) {
            clearInterval(packTimer);
            if (timerElement) {
                timerElement.textContent = "Time's Up!";
            }
        }
    }, 1000);
}

// Handle pack answer verified (sync for non-host players)
function handlePackAnswerVerified(message) {
    console.log('Answer verified:', message);
    
    if (!isHost) {
        // Update score for non-host players
        if (message.isCorrect) {
            currentPackScore++;
        }
        currentPackQuestionIndex++;
        
        setTimeout(() => {
            displayCurrentPackQuestion();
        }, 500);
    }
}

// Handle pack complete
function handlePackComplete(message) {
    console.log('Pack complete:', message);
    
    if (packTimer) {
        clearInterval(packTimer);
    }
    
    const content = document.getElementById('pack-questions-content');
    let html = '<div class="pack-complete-container">';
    html += `<h2>✅ Pack Complete!</h2>`;
    html += `<p class="pack-player">${message.playerName || currentPackPlayer}</p>`;
    html += `<p class="final-score">Final Score: ${message.score || currentPackScore}</p>`;
    
    html += '<div class="button-group">';
    if (isHost) {
        html += '<button class="btn btn-primary btn-large next-turn-btn">Next Player →</button>';
    } else {
        html += '<button class="btn btn-secondary btn-large" disabled>Waiting for host...</button>';
    }
    html += '</div>'
    
    html += '</div>';
    content.innerHTML = html;
    
    // Add event listener for next turn button
    if (isHost) {
        const nextBtn = document.querySelector('.next-turn-btn');
        if (nextBtn) {
            nextBtn.addEventListener('click', endTurn);
        }
    }
}

// End turn function
function endTurn() {
    if (packTimer) {
        clearInterval(packTimer);
    }
    
    sendMessage({
        type: 'end_turn'
    });
    console.log('Ending turn...');
}

// Handle turn ended
function handleTurnEnded(message) {
    console.log('Turn ended:', message);
    // Will receive updated packs list next
}

// Handle Round 2 complete
function handleRound2Complete(message) {
    const content = document.getElementById('round2-complete-content');
    html = `<div class="round-complete-message">
        <h2>🎉 ${message.message}</h2>
        <p>All question packs have been completed!</p>
    </div>`;
    content.innerHTML = html;
    showScreen('round2-complete-screen');
}

// Handle game ended
function handleGameEnded(message) {
    showScreen('game-over');
}

// Update player list
function updatePlayerList() {
    const listElement = document.getElementById('player-list');
    const lobbyListElement = document.getElementById('player-lobby-list');
    const count = document.getElementById('player-count');
    
    if (players.length === 0) {
        listElement.innerHTML = '<p class="empty-message">Waiting for players to join...</p>';
        if (lobbyListElement) lobbyListElement.innerHTML = '';
        if (count) count.textContent = '0';
        document.getElementById('start-btn').disabled = true;
        return;
    }
    
    const html = players.map(p => `
        <div class="player-item">
            <span class="player-name">${p.name}</span>
            <span class="player-status ${p.connected ? 'connected' : 'disconnected'}">
                ${p.connected ? '🟢' : '🔴'}
            </span>
        </div>
    `).join('');
    
    if (listElement) listElement.innerHTML = html;
    if (lobbyListElement) lobbyListElement.innerHTML = html;
    if (count) count.textContent = players.length;
    
    // Enable start button if at least 2 players
    if (isHost && players.length >= 2) {
        document.getElementById('start-btn').disabled = false;
    }
}

// Timer
let timerInterval = null;
function startTimer(seconds) {
    if (timerInterval) clearInterval(timerInterval);
    
    let remaining = seconds;
    const timerElement = document.getElementById('timer');
    timerElement.textContent = remaining;
    
    timerInterval = setInterval(() => {
        remaining--;
        timerElement.textContent = remaining;
        
        if (remaining <= 0) {
            clearInterval(timerInterval);
        }
    }, 1000);
}

// Show screen
function showScreen(screenId) {
    const screens = document.querySelectorAll('.screen');
    screens.forEach(screen => screen.classList.remove('active'));
    
    const targetScreen = document.getElementById(screenId);
    if (targetScreen) {
        targetScreen.classList.add('active');
    }
}

// Show landing
function showLanding() {
    isHost = false;
    currentGamePin = null;
    playerId = null;
    players = [];
    
    if (ws) {
        ws.close();
        ws = null;
    }
    
    showScreen('landing-page');
}

// Show error
function showError(message) {
    alert('Error: ' + message);
}

// Show notification
function showNotification(message) {
    // Simple notification - can be enhanced with better UI
    const notification = document.createElement('div');
    notification.className = 'notification';
    notification.textContent = message;
    document.body.appendChild(notification);
    
    setTimeout(() => {
        notification.remove();
    }, 3000);
}

// Initialize
window.addEventListener('DOMContentLoaded', () => {
    console.log('🎮 Quiz Game initialized');
    showScreen('landing-page');
    
    // Use event delegation on document for all buttons
    document.addEventListener('click', (e) => {
        const target = e.target.closest('button');
        if (!target) return;
        
        // Get the onclick attribute value to determine which function to call
        const onclickAttr = target.getAttribute('onclick');
        
        if (onclickAttr) {
            e.preventDefault();
            
            // Check longer/more specific strings first to avoid conflicts
            if (onclickAttr.includes('showHostGame')) {
                showHostGame();
            } else if (onclickAttr.includes('showJoinGame')) {
                showJoinGame();
            } else if (onclickAttr.includes('showLanding')) {
                showLanding();
            } else if (onclickAttr.includes('startGame')) {
                startGame();
            } else if (onclickAttr.includes('endGame')) {
                endGame();
            } else if (onclickAttr.includes('continueToRound2')) {
                continueToRound2();
            } else if (onclickAttr.includes('nextRound')) {
                // Check nextRound BEFORE nextQuestion to avoid conflict
                nextRound();
            } else if (onclickAttr.includes('nextQuestion')) {
                nextQuestion();
            }
        }
    });
    
    // Handle form submissions
    document.addEventListener('submit', (e) => {
        const form = e.target;
        const onsubmitAttr = form.getAttribute('onsubmit');
        
        if (onsubmitAttr && onsubmitAttr.includes('joinGame')) {
            e.preventDefault();
            joinGame(e);
        }
        
        // Handle speed answer form
        if (form.id === 'speed-answer-form') {
            e.preventDefault();
            submitSpeedAnswer(e);
        }
        
        // Handle tiebreak answer form
        if (form.id === 'tiebreak-answer-form') {
            e.preventDefault();
            submitTiebreakAnswer(e);
        }
    });
});
