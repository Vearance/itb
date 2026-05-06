let board = null;
let positions = [];
let step = 0;
let playTimer = null;
let playDir = 0;

const $ = id => document.getElementById(id);

function App() {
  return window.go.main.App;
}

function tracedCells(currentStep) {
  const cells = new Set();
  if (positions.length === 0 || currentStep === 0) return cells;
  const [r1, c1] = positions[currentStep - 1];
  const [r2, c2] = positions[currentStep];
  const dr = Math.sign(r2 - r1);
  const dc = Math.sign(c2 - c1);
  let r = r1, c = c1;
  cells.add(`${r},${c}`);
  while (r !== r2 || c !== c2) {
    r += dr;
    c += dc;
    cells.add(`${r},${c}`);
  }
  return cells;
}

function classFor(ch) {
  if (ch === 'X') return 'wall';
  if (ch === 'L') return 'lava';
  if (ch === 'O') return 'goal';
  if (ch >= '0' && ch <= '9') return 'num';
  if (ch === 'Z') return 'start';
  return 'empty';
}

function displayChar(ch) {
  if (ch === 'X' || ch === 'L' || ch === 'O' || ch === 'Z' || ch === '*') return '';
  return ch;
}

function render() {
  if (!board) return;
  const t = $('board');
  t.innerHTML = '';
  const trail = tracedCells(step);
  for (let i = 0; i < board.rows; i++) {
    const tr = t.insertRow();
    for (let j = 0; j < board.cols; j++) {
      const td = tr.insertCell();
      const ch = board.grid[i][j];
      td.className = classFor(ch);
      td.textContent = displayChar(ch);

      if (trail.has(`${i},${j}`) && ch !== 'X' && ch !== 'L' && ch !== 'O') {
        td.classList.add('trail');
      }
      if (board.start[0] === i && board.start[1] === j) {
        td.classList.add('start');
      }
      if (positions.length > 0 && positions[step][0] === i && positions[step][1] === j) {
        td.classList.add('actor');
      }
    }
  }
}

function setStepLabel() {
  $('step').textContent = positions.length === 0
    ? '—'
    : `Step ${step} / ${positions.length - 1}`;
}

function updateHeurVisibility() {
  const algo = $('algo').value;
  const needsHeur = algo === 'GBFS' || algo === 'A*' || algo === 'IDA*';
  $('heur-field').style.display = needsHeur ? '' : 'none';
}
$('algo').onchange = updateHeurVisibility;
updateHeurVisibility();

$('browse').onclick = async () => {
  const picked = await App().PickFile();
  if (picked) $('file').value = picked;
};

function showBoard() {
  $('board').hidden = false;
}

function setStats({path, cost, iter, time}) {
  $('r-path').textContent = path ?? '—';
  $('r-cost').textContent = cost ?? '—';
  $('r-iter').textContent = iter ?? '—';
  $('r-time').textContent = time ?? '—';
}

function heurValue() {
  return $('heur').value.split(' ')[0];
}

function stopPlay() {
  if (playTimer !== null) {
    clearInterval(playTimer);
    playTimer = null;
  }
  playDir = 0;
}

function startPlay(dir) {
  stopPlay();
  if (positions.length === 0) return;
  if (dir > 0 && step >= positions.length - 1) return;
  if (dir < 0 && step <= 0) return;
  playDir = dir;
  const sps = parseFloat($('speed').value);
  const interval = 1000 / sps;
  playTimer = setInterval(() => {
    const atEnd = playDir > 0 && step >= positions.length - 1;
    const atStart = playDir < 0 && step <= 0;
    if (atEnd || atStart) { stopPlay(); return; }
    step += playDir;
    setStepLabel();
    render();
  }, interval);
}

$('speed').oninput = () => {
  $('speed-val').textContent = parseFloat($('speed').value).toFixed(1);
  if (playDir !== 0) startPlay(playDir);
};

$('play-fwd').onclick = () => startPlay(1);
$('play-rev').onclick = () => startPlay(-1);
$('pause').onclick = () => stopPlay();

$('load').onclick = async () => {
  const r = await App().LoadFile($('file').value);
  if (r.error) { alert(r.error); return; }
  stopPlay();
  board = r;
  positions = [];
  step = 0;
  setStats({});
  setStepLabel();
  showBoard();
  $('nopath').hidden = true;
  render();
};

$('solve').onclick = async () => {
  if (!board) { alert('Load a map first'); return; }
  stopPlay();
  const r = await App().Solve($('algo').value, heurValue());
  if (r.error) { alert(r.error); return; }
  const timeStr = `${r.elapsedMs.toFixed(3)} ms`;
  if (!r.found) {
    setStats({iter: r.iter, time: timeStr});
    positions = [];
    setStepLabel();
    render();
    $('nopath').hidden = false;
    return;
  }
  $('nopath').hidden = true;
  positions = r.positions;
  step = 0;
  setStats({
    path: r.path,
    cost: r.cost,
    iter: r.iter,
    time: timeStr,
  });
  setStepLabel();
  render();
};

$('prev').onclick = () => {
  if (positions.length === 0 || step === 0) return;
  stopPlay();
  step--;
  setStepLabel();
  render();
};

$('next').onclick = () => {
  if (positions.length === 0 || step >= positions.length - 1) return;
  stopPlay();
  step++;
  setStepLabel();
  render();
};

$('jump').onclick = () => {
  if (positions.length === 0) return;
  stopPlay();
  const n = parseInt($('jumpval').value, 10);
  if (isNaN(n) || n < 0 || n >= positions.length) {
    alert('Step di luar rentang');
    return;
  }
  step = n;
  setStepLabel();
  render();
};

$('save').onclick = async () => {
  const result = await App().Save($('saveval').value);
  $('saveresult').textContent = result;
};
