(function () {
  'use strict';

  const LEVELS = ['A1', 'A2', 'B1', 'B2', 'C1'];
  const LESSONS_PER_LEVEL = 50;
  const GOSPELS = [
    { name: 'Евангелие от Марка', chapters: 16 },
    { name: 'Евангелие от Матфея', chapters: 28 },
    { name: 'Евангелие от Луки', chapters: 24 },
    { name: 'Евангелие от Иоанна', chapters: 21 }
  ];

  const STORAGE_TASKS = 'pol_tasks';
  const STORAGE_ENGLISH = 'pol_english';
  const STORAGE_GAME = 'pol_gamestats';
  const STORAGE_PRAYER = 'pol_prayer_images';
  const STORAGE_NEXT_ID = 'pol_next_id';

  const FIREBASE_DATA_PATH = 'polData';

  let nextTaskId = parseInt(localStorage.getItem(STORAGE_NEXT_ID) || '1', 10);
  let tasks = JSON.parse(localStorage.getItem(STORAGE_TASKS) || '[]');
  let english = JSON.parse(localStorage.getItem(STORAGE_ENGLISH) || '{}');
  let gameStats = JSON.parse(localStorage.getItem(STORAGE_GAME) || '{"xp":0,"level":1,"streak":0,"lastCompletedDate":null}');
  let prayerImages = JSON.parse(localStorage.getItem(STORAGE_PRAYER) || '{}');

  let firebaseDb = null;

  if (window.firebaseConfig && window.firebaseConfig.apiKey && window.firebaseConfig.databaseURL) {
    try {
      firebase.initializeApp(window.firebaseConfig);
      firebaseDb = firebase.database();
      var statusEl = document.getElementById('cloud-status');
      if (statusEl) statusEl.textContent = '☁ Облако вкл';
    } catch (e) {
      if (document.getElementById('cloud-status')) document.getElementById('cloud-status').textContent = '☁ Ошибка облака';
    }
  } else {
    var statusEl = document.getElementById('cloud-status');
    if (statusEl) statusEl.textContent = '☁ Локально';
  }

  function saveToCloud() {
    if (!firebaseDb) return;
    try {
      firebaseDb.ref(FIREBASE_DATA_PATH).set({
        tasks: tasks,
        english: english,
        gameStats: gameStats,
        prayerImages: prayerImages,
        nextId: nextTaskId
      });
    } catch (e) {}
  }

  function loadFromCloudSnapshot(snap) {
    const data = snap.val();
    if (!data) return;
    if (Array.isArray(data.tasks)) { tasks = data.tasks; localStorage.setItem(STORAGE_TASKS, JSON.stringify(tasks)); }
    if (data.english && typeof data.english === 'object') { english = data.english; localStorage.setItem(STORAGE_ENGLISH, JSON.stringify(english)); }
    if (data.gameStats && typeof data.gameStats === 'object') { gameStats = data.gameStats; localStorage.setItem(STORAGE_GAME, JSON.stringify(gameStats)); }
    if (data.prayerImages && typeof data.prayerImages === 'object') { prayerImages = data.prayerImages; localStorage.setItem(STORAGE_PRAYER, JSON.stringify(prayerImages)); }
    if (typeof data.nextId === 'number' && data.nextId > 0) { nextTaskId = data.nextId; localStorage.setItem(STORAGE_NEXT_ID, String(nextTaskId)); }
    renderGameBlock();
    renderTasks();
    renderEnglishLessons();
    renderWordsTable();
    updatePrayerImage();
  }

  function saveTasks() {
    localStorage.setItem(STORAGE_TASKS, JSON.stringify(tasks));
    saveToCloud();
  }
  function saveEnglish() {
    localStorage.setItem(STORAGE_ENGLISH, JSON.stringify(english));
    saveToCloud();
  }
  function saveGame() {
    localStorage.setItem(STORAGE_GAME, JSON.stringify(gameStats));
    saveToCloud();
  }
  function savePrayerImages() {
    localStorage.setItem(STORAGE_PRAYER, JSON.stringify(prayerImages));
    saveToCloud();
  }

  function formatDate(d) {
    const y = d.getFullYear();
    const m = String(d.getMonth() + 1).padStart(2, '0');
    const day = String(d.getDate()).padStart(2, '0');
    return `${y}-${m}-${day}`;
  }

  function todayStr() {
    return formatDate(new Date());
  }

  function getLessonKey(levelIndex, lessonNum) {
    return LEVELS[levelIndex] + '.' + lessonNum;
  }

  function addTaskCompleted() {
    const today = todayStr();
    if (gameStats.lastCompletedDate === today) return;
    if (gameStats.lastCompletedDate) {
      const prev = new Date(gameStats.lastCompletedDate);
      const curr = new Date(today);
      const diffDays = Math.round((curr - prev) / (1000 * 60 * 60 * 24));
      if (diffDays === 1) gameStats.streak++;
      else gameStats.streak = 1;
    } else {
      gameStats.streak = 1;
    }
    gameStats.lastCompletedDate = today;
    gameStats.xp = (gameStats.xp || 0) + 10;
    let totalNeed = 0;
    for (let l = 1; l <= gameStats.level; l++) totalNeed += l * 10;
    while (gameStats.xp >= totalNeed) {
      gameStats.level++;
      totalNeed += gameStats.level * 10;
    }
    saveGame();
    renderGameBlock();
  }

  function renderGameBlock() {
    const level = gameStats.level || 1;
    const xp = gameStats.xp || 0;
    let totalNeed = 0;
    for (let l = 1; l < level; l++) totalNeed += l * 10;
    const xpInLevel = xp - totalNeed;
    const needInLevel = level * 10;

    document.getElementById('level-text').textContent = '⭐ Уровень ' + level;
    const bar = document.getElementById('xp-bar');
    bar.max = needInLevel;
    bar.value = xpInLevel;
    document.getElementById('xp-text').textContent = xpInLevel + ' / ' + needInLevel + ' XP';
    document.getElementById('streak-text').textContent = '🔥 Серия: ' + (gameStats.streak || 0) + ' дн.';
  }

  function renderTasks() {
    const dateVal = document.getElementById('date-picker').value;
    const filtered = tasks.filter(t => t.deadline === dateVal);
    const tbody = document.getElementById('tasks-tbody');
    tbody.innerHTML = '';
    const priorityClass = { 0: 'priority-low', 1: 'priority-medium', 2: 'priority-high' };
    const priorityText = { 0: 'Низкий', 1: 'Средний', 2: 'Высокий' };

    filtered.forEach(t => {
      const tr = document.createElement('tr');
      tr.innerHTML =
        '<td><input type="checkbox" data-id="' + t.id + '" ' + (t.status === 1 ? 'checked' : '') + '></td>' +
        '<td>' + escapeHtml(t.title) + '</td>' +
        '<td>' + escapeHtml(t.description) + '</td>' +
        '<td class="' + priorityClass[t.priority] + '">' + priorityText[t.priority] + '</td>' +
        '<td>' + escapeHtml(t.category || '—') + '</td>';
      tbody.appendChild(tr);
    });

    const dateTitle = document.getElementById('date-title');
    const today = todayStr();
    let label = dateVal;
    if (dateVal === today) label = 'Сегодня (' + dateVal + ')';
    else if (dateVal === formatDate(new Date(Date.now() + 86400000))) label = 'Завтра (' + dateVal + ')';
    dateTitle.textContent = label + ' (' + filtered.length + ' задач)';

    tbody.querySelectorAll('input[type="checkbox"]').forEach(cb => {
      cb.addEventListener('change', function () {
        const id = parseInt(this.dataset.id, 10);
        const task = tasks.find(t => t.id === id);
        if (task) {
          task.status = this.checked ? 1 : 0;
          if (this.checked) addTaskCompleted();
          saveTasks();
          renderTasks();
        }
      });
    });
  }

  function escapeHtml(s) {
    if (!s) return '';
    const div = document.createElement('div');
    div.textContent = s;
    return div.innerHTML;
  }

  function openTaskModal(editTask) {
    const form = document.getElementById('form-task');
    const titleEl = document.getElementById('modal-task-title');
    const idEl = document.getElementById('task-id');
    const dateVal = document.getElementById('date-picker').value;

    if (editTask) {
      titleEl.textContent = 'Редактировать задачу';
      idEl.value = editTask.id;
      document.getElementById('task-title').value = editTask.title;
      document.getElementById('task-desc').value = editTask.description || '';
      document.getElementById('task-deadline').value = editTask.deadline;
      document.getElementById('task-priority').value = editTask.priority;
      document.getElementById('task-category').value = editTask.category || '';
    } else {
      titleEl.textContent = 'Новая задача';
      idEl.value = '';
      form.reset();
      document.getElementById('task-deadline').value = dateVal;
      document.getElementById('task-priority').value = '1';
    }
    document.getElementById('modal-task').classList.add('active');
  }

  function closeTaskModal() {
    document.getElementById('modal-task').classList.remove('active');
  }

  document.getElementById('form-task').addEventListener('submit', function (e) {
    e.preventDefault();
    const id = document.getElementById('task-id').value;
    const title = document.getElementById('task-title').value.trim();
    const description = document.getElementById('task-desc').value.trim();
    const deadline = document.getElementById('task-deadline').value;
    const priority = parseInt(document.getElementById('task-priority').value, 10);
    const category = document.getElementById('task-category').value.trim();

    if (id) {
      const task = tasks.find(t => t.id === parseInt(id, 10));
      if (task) {
        task.title = title;
        task.description = description;
        task.deadline = deadline;
        task.priority = priority;
        task.category = category;
      }
    } else {
      tasks.push({
        id: nextTaskId++,
        title,
        description,
        deadline,
        priority,
        category,
        status: 0,
        createdAt: new Date().toISOString(),
        completedAt: null
      });
      localStorage.setItem(STORAGE_NEXT_ID, String(nextTaskId));
    }
    saveTasks();
    closeTaskModal();
    renderTasks();
  });

  document.getElementById('modal-cancel').addEventListener('click', closeTaskModal);

  document.getElementById('btn-add-task').addEventListener('click', () => openTaskModal(null));

  document.getElementById('date-picker').value = todayStr();
  document.getElementById('date-picker').addEventListener('change', renderTasks);

  // Английский
  const levelSelect = document.getElementById('english-level');
  LEVELS.forEach((name, i) => {
    const opt = document.createElement('option');
    opt.value = i;
    opt.textContent = name;
    levelSelect.appendChild(opt);
  });

  function getCurrentLessonIndex() {
    const levelIndex = parseInt(levelSelect.value, 10);
    const lessonEl = document.querySelector('#english-lessons .lesson-item.active');
    const lessonNum = lessonEl ? parseInt(lessonEl.dataset.num, 10) : 1;
    return (levelIndex * LESSONS_PER_LEVEL) + (lessonNum - 1);
  }

  function getWordsForCurrentLesson() {
    const levelIndex = parseInt(levelSelect.value, 10);
    const lessonEl = document.querySelector('#english-lessons .lesson-item.active');
    const lessonNum = lessonEl ? parseInt(lessonEl.dataset.num, 10) : 1;
    const key = getLessonKey(levelIndex, lessonNum);
    return english[key] || [];
  }

  function setWordsForCurrentLesson(words) {
    const levelIndex = parseInt(levelSelect.value, 10);
    const lessonEl = document.querySelector('#english-lessons .lesson-item.active');
    const lessonNum = lessonEl ? parseInt(lessonEl.dataset.num, 10) : 1;
    const key = getLessonKey(levelIndex, lessonNum);
    english[key] = words;
    saveEnglish();
  }

  function renderEnglishLessons() {
    const levelIndex = parseInt(levelSelect.value, 10);
    const list = document.getElementById('english-lessons');
    list.innerHTML = '';
    for (let i = 1; i <= LESSONS_PER_LEVEL; i++) {
      const div = document.createElement('div');
      div.className = 'lesson-item' + (i === 1 ? ' active' : '');
      div.dataset.num = i;
      div.textContent = i;
      div.addEventListener('click', function () {
        list.querySelectorAll('.lesson-item').forEach(el => el.classList.remove('active'));
        this.classList.add('active');
        renderWordsTable();
      });
      list.appendChild(div);
    }
    renderWordsTable();
  }

  function renderWordsTable() {
    const words = getWordsForCurrentLesson();
    const tbody = document.getElementById('words-tbody');
    tbody.innerHTML = '';
    words.forEach((w, i) => {
      const tr = document.createElement('tr');
      tr.dataset.index = i;
      tr.innerHTML = '<td>' + escapeHtml(w.word) + '</td><td>' + escapeHtml(w.translation) + '</td>';
      tr.addEventListener('click', function () {
        tbody.querySelectorAll('tr').forEach(r => r.classList.remove('selected'));
        this.classList.add('selected');
      });
      tbody.appendChild(tr);
    });
  }

  levelSelect.addEventListener('change', renderEnglishLessons);

  document.getElementById('btn-add-word').addEventListener('click', function () {
    const word = document.getElementById('word-input').value.trim();
    const trans = document.getElementById('trans-input').value.trim();
    if (!word) return;
    const words = getWordsForCurrentLesson();
    words.push({ word, translation: trans });
    setWordsForCurrentLesson(words);
    document.getElementById('word-input').value = '';
    document.getElementById('trans-input').value = '';
    renderWordsTable();
  });

  document.getElementById('btn-remove-word').addEventListener('click', function () {
    const row = document.querySelector('#words-tbody tr.selected');
    if (!row) return;
    const index = parseInt(row.dataset.index, 10);
    const words = getWordsForCurrentLesson();
    words.splice(index, 1);
    setWordsForCurrentLesson(words);
    renderWordsTable();
  });

  // Молитва
  const gospelSelect = document.getElementById('prayer-gospel');
  const chaptersEl = document.getElementById('prayer-chapters');
  const prayerTextEl = document.getElementById('prayer-text');
  const prayerImageBox = document.getElementById('prayer-image-box');

  function getPrayerImageKey() {
    const gospelIndex = gospelSelect.selectedIndex;
    const chEl = document.querySelector('#prayer-chapters .lesson-item.active');
    const ch = chEl ? parseInt(chEl.dataset.ch, 10) : 1;
    return gospelIndex + '_' + ch;
  }

  function renderPrayerChapters() {
    const chapters = parseInt(gospelSelect.options[gospelSelect.selectedIndex].value, 10);
    chaptersEl.innerHTML = '';
    for (let i = 1; i <= chapters; i++) {
      const div = document.createElement('div');
      div.className = 'lesson-item' + (i === 1 ? ' active' : '');
      div.dataset.ch = i;
      div.textContent = 'Глава ' + i;
      div.addEventListener('click', function () {
        chaptersEl.querySelectorAll('.lesson-item').forEach(el => el.classList.remove('active'));
        this.classList.add('active');
        updatePrayerText();
        updatePrayerImage();
      });
      chaptersEl.appendChild(div);
    }
    updatePrayerText();
    updatePrayerImage();
  }

  function updatePrayerText() {
    const name = gospelSelect.options[gospelSelect.selectedIndex].text;
    const chEl = document.querySelector('#prayer-chapters .lesson-item.active');
    const ch = chEl ? chEl.dataset.ch : '1';
    prayerTextEl.textContent = name + '\nГлава ' + ch + '\n\nЗдесь можно разместить текст главы или читать по книге.';
  }

  function updatePrayerImage() {
    const key = getPrayerImageKey();
    const data = prayerImages[key];
    prayerImageBox.innerHTML = '';
    if (data) {
      const img = document.createElement('img');
      img.src = data;
      img.alt = 'Глава';
      prayerImageBox.appendChild(img);
    } else {
      const span = document.createElement('span');
      span.className = 'placeholder-text';
      span.textContent = 'Изображение главы ' + (document.querySelector('#prayer-chapters .lesson-item.active')?.dataset.ch || '1') + '\n(добавьте фото)';
      prayerImageBox.appendChild(span);
    }
  }

  gospelSelect.addEventListener('change', renderPrayerChapters);

  document.getElementById('btn-add-prayer-image').addEventListener('click', function () {
    document.getElementById('prayer-file-input').click();
  });

  document.getElementById('prayer-file-input').addEventListener('change', function () {
    const file = this.files[0];
    if (!file || !file.type.startsWith('image/')) return;
    const reader = new FileReader();
    reader.onload = function () {
      const key = getPrayerImageKey();
      prayerImages[key] = reader.result;
      savePrayerImages();
      updatePrayerImage();
    };
    reader.readAsDataURL(file);
    this.value = '';
  });

  // Экспорт данных (скачать файл для переноса на другое устройство)
  function exportData() {
    const backup = {
      version: 1,
      exportedAt: new Date().toISOString(),
      tasks,
      english,
      gameStats,
      prayerImages,
      nextId: nextTaskId
    };
    const blob = new Blob([JSON.stringify(backup, null, 2)], { type: 'application/json' });
    const a = document.createElement('a');
    a.href = URL.createObjectURL(blob);
    a.download = 'pol_backup_' + formatDate(new Date()) + '.json';
    a.click();
    URL.revokeObjectURL(a.href);
  }

  // Импорт данных (загрузить файл с другого устройства)
  function importData(backup) {
    if (!backup || typeof backup !== 'object') return;
    if (Array.isArray(backup.tasks)) {
      tasks = backup.tasks;
      saveTasks();
    }
    if (backup.english && typeof backup.english === 'object') {
      english = backup.english;
      saveEnglish();
    }
    if (backup.gameStats && typeof backup.gameStats === 'object') {
      gameStats = backup.gameStats;
      saveGame();
    }
    if (backup.prayerImages && typeof backup.prayerImages === 'object') {
      prayerImages = backup.prayerImages;
      savePrayerImages();
    }
    if (typeof backup.nextId === 'number' && backup.nextId > 0) {
      nextTaskId = backup.nextId;
      localStorage.setItem(STORAGE_NEXT_ID, String(nextTaskId));
    }
    renderGameBlock();
    renderTasks();
    renderEnglishLessons();
    renderWordsTable();
    updatePrayerImage();
  }

  document.getElementById('btn-export').addEventListener('click', exportData);

  document.getElementById('btn-import').addEventListener('click', function () {
    document.getElementById('import-file-input').click();
  });

  document.getElementById('import-file-input').addEventListener('change', function () {
    const file = this.files[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = function () {
      try {
        const data = JSON.parse(reader.result);
        importData(data);
        alert('Данные загружены. Задачи, слова, уровень и изображения молитв обновлены.');
      } catch (e) {
        alert('Ошибка: файл не подходит или повреждён. Выберите файл резервной копии (pol_backup_*.json).');
      }
    };
    reader.readAsText(file, 'UTF-8');
    this.value = '';
  });

  // Вкладки
  document.querySelectorAll('.tab').forEach(tab => {
    tab.addEventListener('click', function () {
      document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
      document.querySelectorAll('.panel').forEach(p => p.classList.remove('active'));
      this.classList.add('active');
      const id = 'panel-' + this.dataset.tab;
      document.getElementById(id).classList.add('active');
    });
  });

  // Инициализация
  if (tasks.length === 0) {
    const today = todayStr();
    tasks = [
      { id: nextTaskId++, title: 'Английский', description: 'Практика английского: слова, грамматика или чтение', deadline: today, priority: 1, category: 'Английский', status: 0, createdAt: new Date().toISOString(), completedAt: null },
      { id: nextTaskId++, title: 'Молитва', description: 'Утренняя или вечерняя молитва', deadline: today, priority: 2, category: 'Молитва', status: 0, createdAt: new Date().toISOString(), completedAt: null }
    ];
    localStorage.setItem(STORAGE_NEXT_ID, String(nextTaskId));
    saveTasks();
  }

  renderGameBlock();
  renderTasks();
  renderEnglishLessons();
  renderPrayerChapters();

  if (firebaseDb) {
    firebaseDb.ref(FIREBASE_DATA_PATH).on('value', function (snap) {
      const v = snap.val();
      if (v) loadFromCloudSnapshot(snap);
      else saveToCloud();
    });
  }
})();
