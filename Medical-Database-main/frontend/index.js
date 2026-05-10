
    /* ── Card transitions ── */
const loginCard  = document.getElementById('loginCard');
const createCard = document.getElementById('createCard');

function showCreate() {
  loginCard.classList.add('slide-out-left');
  setTimeout(() => {
    loginCard.classList.add('hidden');
    loginCard.classList.remove('slide-out-left');
    createCard.classList.remove('hidden');
    createCard.style.transform = 'translateX(110%)';
    createCard.style.opacity = '0';
    requestAnimationFrame(() => {
      createCard.style.transition = 'transform 0.4s ease, opacity 0.4s ease';
      createCard.style.transform = 'translateX(0)';
      createCard.style.opacity = '1';
    });
  }, 380);
}  

function showLogin() {
  createCard.style.transition = 'transform 0.4s ease, opacity 0.4s ease';
  createCard.style.transform = 'translateX(110%)';
  createCard.style.opacity = '0';
  setTimeout(() => {
    createCard.classList.add('hidden');
    createCard.style.transform = '';
    createCard.style.opacity = '';
    loginCard.classList.remove('hidden');
    loginCard.style.transform = 'translateX(-110%)';
    loginCard.style.opacity = '0';
    loginCard.style.transition = 'transform 0.4s ease, opacity 0.4s ease';
    requestAnimationFrame(() => {
      loginCard.style.transform = 'translateX(0)';
      loginCard.style.opacity = '1';
    });
  }, 380);
}

  /* ── Helper: show error message ── */
function showError(id, msg) {
  const el = document.getElementById(id);
  el.textContent = msg;
  el.style.display = 'block';
}

/* ── DEV BYPASS — remove before production ── */
async function devLogin(role, username) {
  try {
    const res = await fetch('http://localhost:8848/api/dev-login', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ role: role, username: 'dev_' + role })
    });
    const data = await res.json();
    if (res.ok) {
      sessionStorage.setItem('role', data.role);
      sessionStorage.setItem('username', data.username);
      window.location.href = 'admit.html';
    }
  } catch {
    alert('Could not reach the server.');
  }
}

function hideError(id) {
  document.getElementById(id).style.display = 'none';
}

  /* ── LOGIN — POST JSON to /api/login ── */
document.getElementById('loginForm').addEventListener('submit', async function(e) {
  e.preventDefault();
  hideError('loginError');

  const payload = {
  username: document.getElementById('username').value.trim(),
  password: document.getElementById('password').value
  };

  try {
    const res = await fetch('http://localhost:8848/api/login', {
    method:  'POST',
    headers: {'Content-Type': 'application/json' },
    body:    JSON.stringify(payload)
    });

    const data = await res.json();

    if (res.ok) {
      // Backend returns {"success: true", "role": doctor, "username": jdoe}
      // Redirect to admitting page on success
      sessionStorage.setItem('role', data.role);
      sessionStorage.setItem('username', data.username);
      window.location.href = 'admit.html';
    } else {
        // Backend returns { "error": "Invalid username or password" }
        showError('loginError', data.error || 'Login failed. Please try again.');
      }
  }   catch (err) {
        showError('loginError', 'Could not reach the server. Please try again.');
      }
});

// When the role dropdown changes, show or hide the Patient ID field depending on if "patient" is selected.
document.getElementById('newRole').addEventListener('change', function() {
  const pidField = document.getElementById('patientIDField');
  pidField.style.display = this.value === 'patient' ? '' : 'none';
});

  /* ── REGISTER — POST JSON to /api/register ── */
document.getElementById('registerForm').addEventListener('submit', async function(e) {
  e.preventDefault();
  hideError('registerError');

  const password = document.getElementById('newPassword').value;
  const confirmPassword = document.getElementById('confirmPassword').value;

  if (password !== confirmPassword) {
    showError('registerError', 'Passwords do not match.');
    return;
  }

  const payload = {
    firstName: document.getElementById('newFirst').value.trim(),
    lastName:  document.getElementById('newLast').value.trim(),
    username:  document.getElementById('newUsername').value.trim(),
    role:      document.getElementById('newRole').value,
    password:  password,
    patientID: parseInt(document.getElementById('newPatientID').value) || 0  // Sending the PatientID along with the register request, defaults to 0 if empty.
  };

  try {
    const res = await fetch('http://localhost:8848/api/register', {
    method:  'POST',
    headers: {'Content-Type': 'application/json' },
    body:    JSON.stringify(payload)
    });

    const data = await res.json();

    if (res.ok) {
    // Account created — slide back to login with a success hint
      showLogin();
      setTimeout(() => {
        const hint = document.createElement('div');
        hint.className = 'error';
        hint.style.background = '#f0fff4';
        hint.style.color = '#276749';
        hint.style.borderColor = '#9ae6b4';
        hint.textContent = 'Account created! Please sign in.';
        const form = document.getElementById('loginForm');
        form.appendChild(hint);
        setTimeout(() => hint.remove(), 4000);
      }, 500);
    } else {
        showError('registerError', data.error || 'Registration failed. Please try again.');
      }
  }   catch (err) {
        showError('registerError', 'Could not reach the server. Please try again.');
      }
});
