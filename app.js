(function () {
  "use strict";

  const demo = window.CampusDemoData;
  const storageKey = demo.storageKey;
  const students = demo.students;
  const studentById = new Map(students.map((student) => [student.id, student]));
  const today = getLocalDateISO();

  const form = document.getElementById("checkin-form");
  const select = document.getElementById("student-select");
  const feedback = document.getElementById("checkin-feedback");
  const storageNotice = document.getElementById("storage-notice");
  const attendanceList = document.getElementById("attendance-list");
  const presentCount = document.getElementById("present-count");
  const absentCount = document.getElementById("absent-count");
  const reportDate = document.getElementById("report-date");
  const rosterSize = document.getElementById("roster-size");
  const resetButton = document.getElementById("reset-button");

  let canPersist = true;
  let entries = loadEntries();

  updateStorageNotice();
  reportDate.dateTime = today;
  reportDate.textContent = new Intl.DateTimeFormat(undefined, {
    weekday: "long",
    month: "long",
    day: "numeric",
    year: "numeric"
  }).format(new Date(today + "T12:00:00"));
  rosterSize.textContent = "(" + students.length + ")";
  populateStudentSelect();
  renderReport();

  form.addEventListener("submit", function (event) {
    event.preventDefault();
    const studentId = select.value;
    const student = studentById.get(studentId);

    if (!student) {
      showFeedback("Choose a student from the sample class list.", "error");
      select.setAttribute("aria-invalid", "true");
      select.focus();
      return;
    }

    if (entries.some((entry) => entry.studentId === studentId)) {
      showFeedback(student.name + " is already checked in for this demo day.", "error");
      select.setAttribute("aria-invalid", "true");
      select.focus();
      return;
    }

    entries.push({
      studentId: student.id,
      date: today,
      time: getLocalTime(),
      present: true
    });
    saveEntries();
    renderReport();
    showFeedback(student.name + " checked in successfully.", "success");
    select.value = "";
    select.removeAttribute("aria-invalid");
  });

  select.addEventListener("change", function () {
    select.removeAttribute("aria-invalid");
    if (feedback.textContent) {
      showFeedback("", "");
    }
  });

  resetButton.addEventListener("click", function () {
    const confirmed = window.confirm("Restore the original sample attendance for this demo day? Your changes in this browser will be replaced.");
    if (!confirmed) {
      return;
    }

    entries = makeInitialEntries();
    saveEntries();
    renderReport();
    showFeedback("The original sample attendance has been restored.", "success");
    select.value = "";
    select.removeAttribute("aria-invalid");
  });

  function getLocalDateISO() {
    const now = new Date();
    const year = now.getFullYear();
    const month = String(now.getMonth() + 1).padStart(2, "0");
    const day = String(now.getDate()).padStart(2, "0");
    return year + "-" + month + "-" + day;
  }

  function getLocalTime() {
    const now = new Date();
    return String(now.getHours()).padStart(2, "0") + ":" + String(now.getMinutes()).padStart(2, "0");
  }

  function makeInitialEntries() {
    return demo.initialEntries.map((entry) => ({
      studentId: entry.studentId,
      date: today,
      time: entry.time,
      present: entry.present
    }));
  }

  function loadEntries() {
    try {
      const saved = window.localStorage.getItem(storageKey);
      if (saved === null) {
        return makeInitialEntries();
      }

      const parsed = JSON.parse(saved);
      if (!parsed || parsed.version !== 1 || parsed.date !== today || !Array.isArray(parsed.entries)) {
        return makeInitialEntries();
      }

      const validEntries = [];
      const seen = new Set();
      for (const entry of parsed.entries) {
        if (
          entry &&
          studentById.has(entry.studentId) &&
          entry.date === today &&
          entry.present === true &&
          typeof entry.time === "string" &&
          /^([01]\d|2[0-3]):[0-5]\d$/.test(entry.time) &&
          !seen.has(entry.studentId)
        ) {
          validEntries.push({
            studentId: entry.studentId,
            date: today,
            time: entry.time,
            present: true
          });
          seen.add(entry.studentId);
        }
      }
      return validEntries;
    } catch (error) {
      canPersist = false;
      return makeInitialEntries();
    }
  }

  function saveEntries() {
    if (!canPersist) {
      updateStorageNotice();
      return;
    }

    try {
      window.localStorage.setItem(storageKey, JSON.stringify({
        version: 1,
        date: today,
        entries: entries.map((entry) => ({
          studentId: entry.studentId,
          date: today,
          time: entry.time,
          present: true
        }))
      }));
      updateStorageNotice();
    } catch (error) {
      canPersist = false;
      updateStorageNotice();
    }
  }

  function updateStorageNotice() {
    if (canPersist) {
      storageNotice.hidden = true;
      storageNotice.textContent = "";
      return;
    }
    storageNotice.hidden = false;
    storageNotice.textContent = "Browser storage is unavailable. The demo still works, but changes will last only until you close or reload this page.";
  }

  function populateStudentSelect() {
    for (const student of students) {
      const option = document.createElement("option");
      option.value = student.id;
      option.textContent = student.name + " · " + student.rollNumber;
      select.appendChild(option);
    }
  }

  function renderReport() {
    const presentIds = new Set(entries.map((entry) => entry.studentId));
    presentCount.textContent = String(presentIds.size);
    absentCount.textContent = String(students.length - presentIds.size);
    attendanceList.replaceChildren();

    for (const student of students) {
      const entry = entries.find((item) => item.studentId === student.id);
      const item = document.createElement("li");
      item.className = "attendance-row";

      const identity = document.createElement("div");
      identity.className = "student-identity";
      const avatar = document.createElement("span");
      avatar.className = "student-avatar avatar-" + student.id.slice(-1);
      avatar.setAttribute("aria-hidden", "true");
      avatar.textContent = student.name.split(" ").map((part) => part.charAt(0)).join("");
      const nameBlock = document.createElement("span");
      nameBlock.className = "student-name-block";
      const name = document.createElement("strong");
      name.className = "student-name";
      name.textContent = student.name;
      const roll = document.createElement("span");
      roll.className = "student-roll";
      roll.textContent = student.rollNumber;
      nameBlock.append(name, roll);
      identity.append(avatar, nameBlock);

      const status = document.createElement("span");
      status.className = entry ? "attendance-status status-present" : "attendance-status status-absent";
      status.textContent = entry ? "Present" : "Not checked in";

      const time = document.createElement("span");
      time.className = "attendance-time";
      time.textContent = entry ? formatTime(entry.time) : "—";
      time.setAttribute("aria-label", entry ? "Checked in at " + formatTime(entry.time) : "No check-in time");

      item.append(identity, status, time);
      attendanceList.appendChild(item);
    }
  }

  function formatTime(value) {
    const parts = value.split(":");
    const hour = Number(parts[0]);
    const minute = parts[1];
    const suffix = hour >= 12 ? "PM" : "AM";
    const displayHour = hour % 12 || 12;
    return displayHour + ":" + minute + " " + suffix;
  }

  function showFeedback(message, kind) {
    feedback.textContent = message;
    feedback.className = "feedback" + (kind ? " feedback-" + kind : "");
  }
}());
