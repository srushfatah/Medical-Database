/* ════════════════════════════════════════════
   auth.js  —  shared across every page
   • Checks the user is logged in
   • Exposes role/username to every page
   • Hides nav links the role can't access
   • Provides canAccess() helper
   • Handles logout
   ════════════════════════════════════════════ */

const AUTH = (() => {

  const role     = sessionStorage.getItem('role');
  const username = sessionStorage.getItem('username');

  /* ── If not logged in, kick back to login ── */
  if (!role || !username) {
    window.location.href = 'index.html';
  }

  // Checking if the server-side session is still active. If the server was restarted,
  // the session won't exist anymore, so clear the local storage and go back to login.
  fetch('/api/session')
    .then(r => r.json())
    .then(data => {
      if (!data.loggedIn) {
        sessionStorage.clear();
        window.location.href = 'index.html';
      }
    })
    .catch(() => {
      sessionStorage.clear();
      window.location.href = 'index.html';
    });

  /* ── Role permission map ──
     Each page id maps to which roles can see it.
     'edit' means they can also modify data (not just view).       */
  const PERMISSIONS = {
    navAdmit:   { view: ['admin', 'doctor', 'nurse', 'front_desk'],  edit: ['admin', 'front_desk'] },
    navOrders:  { view: ['admin', 'doctor', 'nurse', 'front_desk'],  edit: ['admin', 'doctor', 'nurse'] },
    navLabs:    { view: ['admin', 'doctor', 'nurse', 'patient'],     edit: ['admin', 'doctor', 'nurse'] },
    navTests:   { view: ['admin', 'doctor', 'nurse', 'patient'],     edit: ['admin', 'doctor', 'nurse'] },
    navBilling: { view: ['admin', 'front_desk', 'patient'],          edit: ['admin', 'front_desk'] },
  };

  /* ── Hide nav links this role cannot view ── */
  function applyNav() {
    Object.entries(PERMISSIONS).forEach(([id, perms]) => {
      const el = document.getElementById(id);
      if (el && !perms.view.includes(role)) {
        el.style.display = 'none';
      }
    });

    /* Show the logged-in user's name + role in the topbar */
    const userDisplay = document.getElementById('userDisplay');
    if (userDisplay) {
      const roleLabel = {
        admin:      'Admin',
        doctor:     'Doctor',
        nurse:      'Nurse',
        patient:    'Patient',
        front_desk: 'Front Desk'
      }[role] || role;
      userDisplay.textContent = `${username}  (${roleLabel})`;
    }
  }

  /* ── canAccess(navId, mode)
     mode = 'view' (default) or 'edit'
     Returns true if the current role has that permission.         */
  function canAccess(navId, mode = 'view') {
    const perms = PERMISSIONS[navId];
    if (!perms) return false;
    return perms[mode].includes(role);
  }

  /* ── Show a "read-only" banner on sections this role can't edit ── */
  function markReadOnly(sectionId) {
    const section = document.getElementById(sectionId);
    if (!section) return;
    if (section.querySelector('.readonly-banner')) return;
    const banner = document.createElement('div');
    banner.className = 'readonly-banner';
    banner.textContent = '👁 View Only — your role cannot modify this section';
    section.prepend(banner);
    /* Disable all inputs/buttons inside */
    section.querySelectorAll('input, select, textarea, button[type="submit"]')
           .forEach(el => { el.disabled = true; el.style.opacity = '0.5'; });
  }

  /* ── Logout ── */
  function logout() {
    sessionStorage.clear();
    window.location.href = 'index.html';
  }

  /* ── Run nav setup after DOM is ready ── */
  document.addEventListener('DOMContentLoaded', applyNav);

  /* Public API */
  return { role, username, canAccess, markReadOnly, logout };

})();
