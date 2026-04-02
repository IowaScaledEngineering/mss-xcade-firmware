document.getElementById('downloadConfig').addEventListener('click', async () => {
  try {
    // 1. Fetch the data from your endpoint
    const response = await fetch('/api/loadConfig');
    
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    const data = await response.json();

    // 2. Convert the JSON object to a String
    const jsonString = JSON.stringify(data, null, 2);

    // 3. Create a Blob from the JSON string
    const blob = new Blob([jsonString], { type: 'application/json' });

    // 4. Create a temporary URL for the Blob
    const url = window.URL.createObjectURL(blob);

    // 5. Create a hidden anchor element to trigger the download
    const link = document.createElement('a');
    link.href = url;
    link.download = 'config.json'; // The default filename
    
    // Append to body, click it, and remove it
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);

    // 6. Clean up the URL object to free up memory
    window.URL.revokeObjectURL(url);

  } catch (error) {
    console.error('Failed to download config:', error);
    alert('Error downloading configuration file.');
  }
});

document.addEventListener('DOMContentLoaded', () => {
  // Call the load function as soon as the page is ready
  loadConfiguration();
});

async function loadConfiguration() {
  let configData = {};

  // --- REAL FETCH LOGIC ---
  try {
    const response = await fetch('/api/loadConfig');
    if (!response.ok) throw new Error("Failed to load config");
    configData = await response.json();
  } catch (error) {
    console.error("Error loading configuration:", error);
    return; // Stop if we can't get data
  }

  // Apply the loaded data to the DOM
  applyConfigToForm(configData, 'config-tab');
}

function applyConfigToForm(data, tabContainerId) {
  // 1. Get the specific tab container
  const container = document.getElementById(tabContainerId);
  
  // Safety check: if the tab doesn't exist, stop
  if (!container) {
    console.error("Tab container not found:", tabContainerId);
    return;
  }

  // 2. Loop through every key/value pair in the JSON
  for (const [key, value] of Object.entries(data)) {
    
    // 3. Find inputs matching the name ONLY inside the specified container
    const inputs = container.querySelectorAll(`[name="${key}"]`);
    
    if (inputs.length === 0) continue; // Skip if no matching input exists in this tab

    inputs.forEach(input => {
      if (input.type === 'checkbox') {
        // Checkboxes expect a boolean
        input.checked = !!value; 
      } 
      else if (input.type === 'radio') {
        // For radio groups, only check the one that matches the specific value
        if (input.value == value) {
            input.checked = true;
        }
      } 
      else {
        // Standard text inputs, number boxes, and select dropdowns
        input.value = value;
        
        // CRITICAL: Manually trigger the events so visual sliders/custom UI update
        input.dispatchEvent(new Event('input', { bubbles: true }));
        input.dispatchEvent(new Event('change', { bubbles: true }));
      }
    });
  }
}

async function submitBasicConfig() {
  // 1. Confirmation Dialog
  const confirmed = confirm("Are you sure you want to change the configuration? The node will restart.");
  if (!confirmed) {
    return; // Stop if user clicks Cancel
  }

  const container = document.getElementById('home-tab');

  // 2. Find all relevant inputs inside that div
  const inputs = container.querySelectorAll('input, select, textarea');

  const data = {};

  inputs.forEach(input => {
    // Only include fields that have a 'name' attribute
    if (input.name) {
      // Special handling for checkboxes
      if (input.type === 'checkbox') {
        data[input.name] = input.checked; 
      } 
      // Special handling for radio buttons (only grab the checked one)
      else if (input.type === 'radio') {
        if (input.checked) {
            data[input.name] = input.value;
        }
      } 
      // Standard text inputs, selects, etc.
      else {
        data[input.name] = input.value;
      }
    }
  });

  try {
    // 4. Send Data via Fetch
    const response = await fetch('/api/saveBasicConfig', {
      method: 'POST',
      headers: {
          'Content-Type': 'application/json'
      },
      body: JSON.stringify(data)
    });

    // 5. Handle Response
    if (response.ok) {
      alert("Configuration saved! Restarting...");
      // Optional: reload page
      // location.reload(); 
    } else {
      alert("Error saving: " + response.status);
    }
  } catch (error) {
    console.error("Network error:", error);
    alert("Failed to connect to the device.");
  }
}

async function submitConfig() {
  const container = document.getElementById('config-tab');

  // 2. Find all relevant inputs inside that div
  const inputs = container.querySelectorAll('input, select, textarea');

  const data = {};

  inputs.forEach(input => {
    // Only include fields that have a 'name' attribute
    if (input.name) {
      // Special handling for checkboxes
      if (input.type === 'checkbox') {
        data[input.name] = input.checked; 
      } 
      // Special handling for radio buttons (only grab the checked one)
      else if (input.type === 'radio') {
        if (input.checked) {
            data[input.name] = input.value;
        }
      } 
      // Standard text inputs, selects, etc.
      else {
        data[input.name] = input.value;
      }
    }
  });

  try {
    // 4. Send Data via Fetch
    const response = await fetch('/api/saveConfig', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify(data)
    });

    // 5. Handle Response
    if (response.ok) {
      // location.reload(); 
    } else {
      alert("Error saving: " + response.status);
    }
  } catch (error) {
    console.error("Network error:", error);
    alert("Failed to connect to the device.");
  }
}

document.addEventListener('DOMContentLoaded', () => {
  // Start polling every 500ms
  setInterval(updateStatus, 500);
});

async function updateStatus() {
  // 1. VISIBILITY CHECK
  // specific to the tab logic in your xcade-status.html file
  const statusTab = document.getElementById('status-tab');
  
  // If the tab is missing or doesn't have the 'active' class, stop here.
  if (!statusTab || !statusTab.classList.contains('active')) {
    return;
  }


  try {
    // 1. Fetch data from your API
    const response = await fetch('/api/getStatus');
    
    if (!response.ok) {
      // Silently fail or log to console if network is down
      console.warn("Status fetch failed:", response.status);
      return;
    }

    const data = await response.json();

    // Iterate over the flat JSON keys (e.g., "mss1a-status", "mss1a-s-in")
    for (const [elementId, value] of Object.entries(data)) {
      
      const element = document.getElementById(elementId);
      const statusTab = document.getElementById('status-tab');

      // Ensure element exists and is inside the Status Tab
      if (element && statusTab.contains(element)) {
        // METHOD 1: Text Updates
        // Check if the ID ends with "-status" (e.g., "mss1a-status")
        if (elementId.endsWith('-statustxt')) {
          // Use innerHTML to allow line breaks <br> sent from server
          element.innerHTML = value;
        } 
        
        // METHOD 2: Color Toggling (Signal Boxes)
        // Default behavior for all other IDs (s-in, a-out, etc.)
        else {
          element.classList.remove('wireActive', 'wireInactive');
          // Logic: 1 = Active (Orange/Green class), 0 = Inactive (Blue/Red class)
          if (value) {
            element.classList.add('wireActive');
          } else {
            element.classList.add('wireInactive');
          }
        }
      }
    }
  } catch (error) {
    console.error("Polling error:", error);
  }
}


document.addEventListener('DOMContentLoaded', () => {
  const tabs = document.querySelectorAll('.tab-btn');
  const allContent = document.querySelectorAll('.tab-content');
  tabs.forEach((tab) => {
    tab.addEventListener('click', () => {
        tabs.forEach(t => t.classList.remove('active'));
        allContent.forEach(c => c.classList.remove('active'));
        tab.classList.add('active');
        const contentId = tab.getAttribute('data-tab');
        const contentToShow = document.getElementById(contentId);
        if(contentToShow) {
        contentToShow.classList.add('active');
        }
    });
  });
});
