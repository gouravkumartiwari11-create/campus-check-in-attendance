/* Fictional records for this local attendance demonstration. */
window.CampusDemoData = Object.freeze({
  storageKey: "campus-check-in-demo-v1",
  students: Object.freeze([
    Object.freeze({ id: "stu-101", rollNumber: "C-101", name: "Alex Rivera" }),
    Object.freeze({ id: "stu-102", rollNumber: "C-102", name: "Jamie Chen" }),
    Object.freeze({ id: "stu-103", rollNumber: "C-103", name: "Morgan Patel" }),
    Object.freeze({ id: "stu-104", rollNumber: "C-104", name: "Taylor Okafor" }),
    Object.freeze({ id: "stu-105", rollNumber: "C-105", name: "Casey Nguyen" }),
    Object.freeze({ id: "stu-106", rollNumber: "C-106", name: "Riley Brooks" })
  ]),
  initialEntries: Object.freeze([
    Object.freeze({ studentId: "stu-101", time: "08:05", present: true }),
    Object.freeze({ studentId: "stu-103", time: "08:12", present: true }),
    Object.freeze({ studentId: "stu-106", time: "08:18", present: true })
  ])
});
