// Constants
const modes = { USER: 0, KNOWN: 1, BLACK: 2, BLACKKNOWN: 3 };
const colors = {
  WRONG: '#b22222', SOLUTION: '#cc9900',
  USER: '#003366', KNOWN: '#000000',
  BUTTONDOWN: '#557055', BUTTONUP: '#8fbc8f',
  WHITE: '#ffffff', BLACK: '#000000',
  FIELDSELECTED: '#cfe2cf', FIELDUNSELECTED: '#ffffff'
};
const difficulties = ["Ultra", "Sehr schwer", "Schwer", "Mittel", "Leicht", "Sehr leicht"];

// Variables
var starttime;
var timer = undefined;
var count = 0;
var noteMode = false;
var game;
var activeRow = undefined, activeCol = undefined;
var showSolution = false;
var actionHistory = [];
var gameCode = undefined;
var gameUrl = undefined;
var difficulty = 3;

// Element class
class Field{
  constructor (row, col) {
    if (col === undefined) {
      this.selector = row;
    } else {
      this.selector = `#ce${row}${col}`;
    }
    this.value = undefined;
    this.user = undefined;
    this.notes = [];
    this.mode = undefined;
    this.wrong = false;
    this.solution = false;
  }
  setUser (input) {
    if (this.mode === modes.USER) {
      this.wrong = false;
      this.notes = [];
      if (this.user === input) {
        this.user = undefined;
      } else {
        this.user = input;
      }
      this.render();
    }
  }
  setNote (value) {
    if (this.mode === modes.USER) {
      this.user = undefined;
      if (this.notes.indexOf(value) > -1) {
        this.notes.splice(this.notes.indexOf(value), 1);
      } else {
        this.notes.push(value);
      }
      this.render();
    }
  }
  checkUser (setColor) {
    if (this.mode !== modes.USER) return true;
    if (!this.user) return false;
    if (this.user === this.value) return true;
    if (setColor) {
      this.wrong = true;
      this.render();
    }
    return false;
  }
  showSolution () {
    this.solution = true;
    this.render();
  }
  restart () {
    this.user = undefined;
    this.notes = [];
    this.render();
  }
  copy () {
    let field = new Field(this.selector);
    field.mode = this.mode;
    field.value = this.value;
    field.user = this.user;
    field.wrong = this.wrong;
    field.notes = [...this.notes];
    return field;
  }
  getElement() {
    return $(this.selector);
  }
  reset () {
    this.getElement().empty();
    this.getElement().css('background-color', colors.WHITE);
  }
  render () {
    this.getElement().empty();
    if (this.mode === modes.USER) {
      if (this.solution) {
        if (this.user === this.value) {
          this.getElement().css('color', colors.SOLUTION);
        } else {
          this.getElement().css('color', colors.WRONG);
        }
        this.getElement().text(this.value);
      } else {
        if (this.notes.length > 0) {
          this.getElement().css('color', colors.USER);
          var notes = '<table class="mini" cellspacing="0">';
          for (let i = 1; i < 10; i++) {
            if ((i - 1) % 3 === 0) notes += '<tr>';
            if (this.notes.indexOf(i) >= 0) {
              notes += `<td>${i}</td>`;
            } else {
              notes += `<td class="transparent">${i}</td>`;
            }
            if (i % 3 === 0) notes += '</tr>';
          }
          notes += '</table>';
          this.getElement().append(notes);
        } else if (this.user) {
          if (this.wrong) {
            this.getElement().css('color', colors.WRONG);
          } else {
            this.getElement().css('color', colors.USER);
          }
          this.getElement().text(this.user);
        }
      }
    } else if (this.mode === modes.BLACKKNOWN) {
      this.getElement().css('color', colors.WHITE);
      this.getElement().css('background-color', colors.BLACK);
      this.getElement().text(this.value);
    } else if (this.mode === modes.KNOWN) {
      this.getElement().css('background-color', colors.WHITE);
      this.getElement().css('color', colors.KNOWN);
      this.getElement().text(this.value);
    } else {
      this.getElement().css('background-color', colors.BLACK);
    }
  }
}

// class to store and modify the current game state
class Game {
  constructor() {
    this.data = [];
    for (let r = 0; r < 9; r++) {
      this.data.push([]);
      for (let c = 0; c < 9; c++) {
        this.data[r].push(new Field());
      }
    }
  }
  get(row, col) {
    return this.data[row][col];
  }
  setValues(row, col, mode, value) {
    this.data[row][col] = new Field(row, col);
    this.data[row][col].mode = mode;
    this.data[row][col].value = value;
    this.data[row][col].render();
  }
  setField(field) {
    const row = Number(field.selector.substring(3, 4));
    const col = Number(field.selector.substring(4, 5));
    this.data[row][col] = field;
    this.data[row][col].render();
  }
  forEach (iteratorFunction) {
    for (let r = 0; r < 9; r++) {
      for (let c = 0; c < 9; c++) {
        iteratorFunction(this.data[r][c], r, c);
      }
    }
  }
}

// Button Functions
function restart () {
  if (!showSolution) {
    game.forEach(field => {
      field.restart();
    })
  }
}
function toggleNoteMode () {
  noteMode = !noteMode;
  const color = (noteMode) ? colors.BUTTONDOWN : colors.BUTTONUP
  $('#notes').css('background-color', color);
}
function check() {
  if (!showSolution) {
    count++;
    $('#counter').text(count);
    game.forEach(field => {
      field.checkUser(setColor = true);
    })
  }
}
function solution () {
  showSolution = true;
  clearInterval(timer);
  game.forEach(field => {
    field.showSolution();
  })
}
function back () {
  if (actionHistory.length > 0 && !showSolution) {
    const field = actionHistory.pop();
    game.setField(field);
  }
}

// Parse game
function parseGame (code) {
  game = new Game();
  const base64urlCharacters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_';
  var binary = '';
  for (let i = 0; i < code.length; i++) {
    b = base64urlCharacters.indexOf(code.charAt(i)).toString(2);
    while (b.length < 6) b = '0' + b;
    binary += b;
  }
  const encodingVersion = parseInt(binary.substring(0, 8), 2);
  binary = binary.substring(8);
  switch (encodingVersion) {
    default:
      if (binary.length < (6 * 81)) return; // Invalid data
      for (let i = 0; i < 81; i++) {
        const subBinary = binary.substring(i * 6, (i + 1) * 6);
        const mode = parseInt(subBinary.substring(0, 2), 2);
        const value = parseInt(subBinary.substring(2, 6), 2) + 1;
        game.setValues(Math.floor(i / 9), i % 9, mode, value);
      }
      binary = binary.substring(6 * 81);
      let counter = 0;
      while (binary.length >= 7 && counter < difficulty * 3.5) {
        const position = parseInt(binary.substring(0, 7), 2);
        game.get(Math.floor(position / 9), position % 9).mode = modes.KNOWN;
        game.get(Math.floor(position / 9), position % 9).render();
        binary = binary.substring(7);
        counter++;
      }
  }
}

// General Functions
function setup () {
  for (let r = 0; r < 9; r++) {
    var row = `<tr class="row" id="r${r}" row="${r}">`;
    for (let c = 0; c < 9; c++) {
      row += `<td class="cell" id="ce${r}${c}" row="${r}" col="${c}"></td>`;
    }
    row += '</tr>';
    $('.container').append(row);
  }
}
function restartTimer () {
  starttime = (new Date()).getTime();
  timer = setInterval(function () {
    const diff = (new Date()).getTime() - starttime;
    const minutes = Math.floor(diff / 60000);
    const seconds = Math.round(diff / 1000 - minutes * 60);
    $('#time').text(((minutes < 10) ? '0' : '') + minutes + ':' + ((seconds < 10) ? '0' : '') + seconds);
  }, 1000);
}
function getURLParameter (name) {
  return decodeURIComponent((new RegExp('[?|&]' + name + '=' + '([^&;]+?)(&|#|;|$)').exec(location.search) || [null, ''])[1].replace(/\+/g, '%20')) || null;
}
function loadNewGame () {
  dialogVisibility(true, true);
  clearInterval(timer);
  $.get('https://luiswalter.me/str8ts/getGame', data => {
    if (data.length > 82) {
      console.log("Game:", data);
      gameUrl = window.location.href.split('?')[0] + '?code=' + data
      gameCode = data;
      dialogVisibility(true, false);
    } else {
      console.log(data);
      loadNewGame();
    }
  })
}

function changeDifficulty () {
  difficulty = Number($('#difficultySlider').val());
  $('#difficulty').text(difficulties[difficulty]);
}

function copyGameURL () {
  $('#share-game-url').val(gameUrl);
  $('#share-game-url').focus();
  $('#share-game-url').select();
  document.execCommand('copy');
}

function startGame () {
  if (gameCode && gameCode.length > 82) {
    showSolution = false;
    actionHistory = [];
    activeCol = undefined;
    activeRow = undefined;
    count = 0;
    $('#counter').text(count);
    $(".container").removeClass('finished');
    dialogVisibility(false, false);
    if (game) {
      game.forEach(field => {
        field.reset();
      })
    }
    parseGame(gameCode);
    restartTimer();
  } else {
    loadNewGame();
  }
}

function dialogVisibility (visible, loading) {
  if (visible) {
    $('.dialogOuterContainer').show();
    if (loading) {
      $('#welcome-dialog').hide();
      $('#loading').show();
    } else {
      $('#welcome-dialog').show();
      $('#loading').hide();
      $('#share-game-url').val(gameUrl);
      window.history.replaceState(null, 'Str8ts', gameUrl);
    }
  } else {
    $('.dialogOuterContainer').hide();
  }
}

$(document).ready(function(){
  setup();
  onResize();
  const code = getURLParameter('code');
  if (code && code.length > 82) {
    gameUrl = window.location.href;
    gameCode = code;
    dialogVisibility(true, false);
  } else {
    loadNewGame();
  }
  $('td[id^="ce"]').click(function () { // Game fields
    const row = Number($(this).attr('row'));
    const col = Number($(this).attr('col'));
    if (!showSolution && game.get(row, col).mode == modes.USER) {
      if (typeof activeRow !== 'undefined') {
        game.get(activeRow, activeCol).getElement().css('background-color', colors.FIELDUNSELECTED); // Reset previously selected field
      }
      activeRow = row;
      activeCol = col;
      game.get(activeRow, activeCol).getElement().css('background-color', colors.FIELDSELECTED); // Change background of just selected field
    }
  })
  $('td[id^="bn"]').click(function () { // Number buttons
    if (!showSolution && typeof activeRow !== 'undefined' && game.get(activeRow, activeCol).mode == modes.USER) {
      actionHistory.push(game.get(activeRow, activeCol).copy());
      const num = Number($(this).text());
      if (noteMode) {
        game.get(activeRow, activeCol).setNote(num);
      } else {
        game.get(activeRow, activeCol).setUser(num);
        let finished = true;
        game.forEach(field => {
          if (!field.checkUser(setColor = false)) finished = false;
        })
        if (finished) {
          showSolution = true;
          $(".container").addClass('finished');
          clearInterval(timer);
        } 
      }
    }
  })
})

$(window).resize(onResize);
function onResize() {
  if (window.innerWidth/2 < $('.controls').position().left) { // Large screen
    $('#buttons-small').hide();
    $('#buttons-large').show();
  } else { // Small screen
    $('#buttons-small').show();
    $('#buttons-large').hide();
  }
}