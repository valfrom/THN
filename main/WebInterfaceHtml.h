#pragma once

#include <pgmspace.h>

namespace interface {

static const char kWebInterfaceHtml[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8" />
    <title>HVAC Controller</title>
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <style>
      body {
        font-family: Arial, sans-serif;
        margin: 0 auto;
        padding: 1rem;
        max-width: 960px;
        background: #f4f7f9;
        color: #222;
      }
      header {
        display: flex;
        justify-content: space-between;
        align-items: center;
        margin-bottom: 1.5rem;
      }
      h1 {
        margin: 0;
      }
      section {
        background: #fff;
        border-radius: 8px;
        padding: 1rem;
        margin-bottom: 1rem;
        box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1);
      }
      label {
        display: block;
        font-weight: bold;
        margin-top: 0.5rem;
      }
      input,
      select,
      textarea,
      button {
        width: 100%;
        padding: 0.5rem;
        margin-top: 0.25rem;
        box-sizing: border-box;
        border: 1px solid #ccd6dd;
        border-radius: 4px;
      }
      button {
        background: #0078d4;
        color: #fff;
        border: none;
        cursor: pointer;
      }
      button:disabled {
        background: #aac6eb;
        cursor: not-allowed;
      }
      .grid {
        display: grid;
        grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
        gap: 1rem;
      }
      table {
        width: 100%;
        border-collapse: collapse;
      }
      th,
      td {
        border-bottom: 1px solid #e5e8eb;
        padding: 0.4rem 0.6rem;
        text-align: left;
        font-size: 0.9rem;
      }
      th {
        background: #f0f3f5;
      }
      .status {
        display: flex;
        flex-wrap: wrap;
        gap: 1rem;
      }
      .status div {
        min-width: 140px;
      }
      .log-container {
        max-height: 240px;
        overflow-y: auto;
      }
      .inline {
        display: inline-flex;
        align-items: center;
        gap: 0.5rem;
      }
      .header-meta {
        display: flex;
        flex-direction: column;
        align-items: flex-end;
        gap: 0.25rem;
        text-align: right;
      }
      .header-meta div {
        font-size: 0.9rem;
      }
    </style>
  </head>
  <body>
    <header>
      <h1>HVAC Controller</h1>
      <div class="header-meta">
        <div id="wifiInfo"></div>
        <div id="currentTime">-</div>
      </div>
    </header>

    <section id="statusSection">
      <h2>Current Status</h2>
      <div class="status">
        <div>Mode: <span id="systemMode">-</span></div>
        <div>Fan: <span id="fanMode">-</span> (<span id="fanSpeed">-</span>)</div>
        <div>Compressor: <span id="compressor">-</span></div>
        <div>Compressor Timeout: <span id="compressorTimeout">-</span></div>
        <div>Compressor Off Timeout: <span id="compressorOffTimeout">-</span></div>
        <div>Cooldown Active: <span id="compressorCooldown">-</span></div>
        <div>Cooldown Remaining: <span id="compressorCooldownRemaining">-</span></div>
        <div>Target: <span id="target">-</span>°C</div>
        <div>Hysteresis: <span id="hysteresis">-</span>°C</div>
        <div>Compressor Temp Limit: <span id="compressorTempLimit">-</span>°C</div>
        <div>Compressor Min Ambient: <span id="compressorMinAmbient">-</span>°C</div>
        <div>Cooldown Temp: <span id="compressorCooldownTemp">-</span>°C</div>
        <div>Cooldown Duration: <span id="compressorCooldownMinutes">-</span> min</div>
        <div>Ambient: <span id="ambient">-</span>°C</div>
        <div>Coil: <span id="coil">-</span>°C</div>
        <div>Energy: <span id="energy">-</span> Wh</div>
      </div>
    </section>

    <section>
      <h2>Configuration</h2>
      <form id="configForm">
        <div class="grid">
          <div>
            <label for="targetInput">Target Temperature (°C)</label>
            <input id="targetInput" name="target" type="number" step="0.1" />
          </div>
          <div>
            <label for="hysteresisInput">Hysteresis (°C)</label>
            <input id="hysteresisInput" name="hysteresis" type="number" step="0.1" />
          </div>
          <div>
            <label for="compressorTempLimitInput">Compressor Temp Limit (°C)</label>
            <input
              id="compressorTempLimitInput"
              name="compressorTempLimit"
              type="number"
              step="0.1"
              min="0"
            />
          </div>
          <div>
            <label for="compressorMinAmbientInput">Compressor Min Ambient (°C)</label>
            <input
              id="compressorMinAmbientInput"
              name="compressorMinAmbient"
              type="number"
              step="0.1"
            />
          </div>
          <div>
            <label for="compressorCooldownTempInput">Compressor Cooldown Temp (°C)</label>
            <input
              id="compressorCooldownTempInput"
              name="compressorCooldownTemp"
              type="number"
              step="0.1"
              min="0"
            />
          </div>
          <div>
            <label for="compressorCooldownMinutesInput">Compressor Cooldown Duration (minutes)</label>
            <input
              id="compressorCooldownMinutesInput"
              name="compressorCooldownMinutes"
              type="number"
              step="0.1"
              min="0"
            />
          </div>
          <div>
            <label for="fanModeInput">Fan Mode</label>
            <select id="fanModeInput" name="fanMode">
              <option value="auto">Automatic</option>
              <option value="off">Off</option>
              <option value="low">Low</option>
              <option value="medium">Medium</option>
              <option value="high">High</option>
            </select>
          </div>
          <div>
            <label for="systemModeInput">System Mode</label>
            <select id="systemModeInput" name="systemMode">
              <option value="cooling">Cooling</option>
              <option value="heating">Heating</option>
              <option value="fan">Fan Only</option>
              <option value="idle">Idle</option>
            </select>
          </div>
          <div>
            <label for="timezoneOffsetInput">Timezone Offset (hours from UTC)</label>
            <input
              id="timezoneOffsetInput"
              name="timezoneOffset"
              type="number"
              step="0.25"
              min="-12"
              max="14"
            />
          </div>
        </div>
        <label class="inline">
          <input id="schedulingInput" name="scheduling" type="checkbox" value="true" />
          Enable scheduling
        </label>
        <label for="weekdayInput">
          Weekday Schedule (HH:MM=TEMP|mode;… – mode optional, e.g. cooling/idle)
        </label>
        <textarea
          id="weekdayInput"
          name="weekday"
          rows="3"
          placeholder="06:00=23.0|cooling;09:00=26.0|cooling;17:30=23.5|cooling;22:00=25.0|idle"
        ></textarea>
        <label for="weekendInput">
          Weekend Schedule (HH:MM=TEMP|mode;… – mode optional, e.g. cooling/idle)
        </label>
        <textarea
          id="weekendInput"
          name="weekend"
          rows="3"
          placeholder="08:00=23.5|cooling;12:00=25.0|cooling;18:00=23.0|cooling;23:00=25.5|idle"
        ></textarea>
        <button type="submit">Save Configuration</button>
        <p id="configStatus"></p>
      </form>
    </section>

    <section>
      <h2>Logs</h2>
      <div class="grid">
        <div>
          <h3>Temperature Log</h3>
          <div class="log-container">
            <table>
              <thead>
                <tr>
                  <th>Time (ms)</th>
                  <th>Ambient (°C)</th>
                  <th>Coil (°C)</th>
                </tr>
              </thead>
              <tbody id="temperatureLog"></tbody>
            </table>
          </div>
        </div>
        <div>
          <h3>Power Log</h3>
          <div class="log-container">
            <table>
              <thead>
                <tr>
                  <th>Time (ms)</th>
                  <th>Watts</th>
                  <th>Wh</th>
                  <th>Fan</th>
                  <th>Compressor</th>
                </tr>
              </thead>
              <tbody id="powerLog"></tbody>
            </table>
          </div>
        </div>
      </div>
    </section>

    <script>
      const configForm = document.getElementById('configForm');
      const configStatus = document.getElementById('configStatus');
      const timezoneOffsetInput = document.getElementById('timezoneOffsetInput');
      const detectedTimezoneOffsetHours = -new Date().getTimezoneOffset() / 60;
      let timezoneOffsetAutoApplied = false;
      const systemModeLabels = {
        cooling: 'Cooling',
        heating: 'Heating',
        fan: 'Fan Only',
        idle: 'Idle',
      };
      const fanModeLabels = {
        auto: 'Auto',
        off: 'Off',
        low: 'Low',
        medium: 'Medium',
        high: 'High',
      };
      const fanSpeedLabels = {
        off: 'Off',
        low: 'Low',
        medium: 'Medium',
        high: 'High',
      };
      let currentTimeState = {
        epochSeconds: null,
        receivedAtMs: null,
        formatted: null,
      };

      async function fetchState() {
        const response = await fetch('/api/state');
        if (!response.ok) {
          throw new Error('Failed to load state');
        }
        return await response.json();
      }

      function scheduleToText(entries) {
        return entries
          .map((entry) => {
            const time = entry.time;
            const temp = Number(entry.temp).toFixed(1);
            const mode = typeof entry.mode === 'string' && entry.mode.length > 0 ? `|${entry.mode}` : '';
            return `${time}=${temp}${mode}`;
          })
          .join(';');
      }

      function renderLogs(element, entries, fields) {
        element.innerHTML = entries
          .map((entry) => {
            return `<tr>${fields
              .map((field) => `<td>${entry[field] ?? '-'}</td>`)
              .join('')}</tr>`;
          })
          .join('');
      }

      function formatTimestamp(epochSeconds) {
        const numeric = Number(epochSeconds);
        if (!Number.isFinite(numeric)) {
          return '-';
        }
        const date = new Date(numeric * 1000);
        const pad = (value) => String(value).padStart(2, '0');
        const year = date.getFullYear();
        const month = pad(date.getMonth() + 1);
        const day = pad(date.getDate());
        const hours = pad(date.getHours());
        const minutes = pad(date.getMinutes());
        const seconds = pad(date.getSeconds());
        return `${year}-${month}-${day} ${hours}:${minutes}:${seconds}`;
      }

      function toFixedOrDash(value, digits = 1) {
        const numeric = Number(value);
        return Number.isFinite(numeric) ? numeric.toFixed(digits) : '-';
      }

      function formatOffsetHours(value) {
        const numeric = Number(value);
        if (!Number.isFinite(numeric)) {
          return '';
        }
        const rounded = Math.round(numeric * 100) / 100;
        if (Number.isInteger(rounded)) {
          return String(rounded);
        }
        return rounded.toFixed(2).replace(/0+$/, '').replace(/\.$/, '');
      }

      if (timezoneOffsetInput) {
        const detectedFormatted = formatOffsetHours(detectedTimezoneOffsetHours);
        if (detectedFormatted !== '') {
          timezoneOffsetInput.placeholder = detectedFormatted;
          timezoneOffsetInput.title = `Detected from browser: ${detectedFormatted} hours from UTC`;
          timezoneOffsetInput.value = detectedFormatted;
        }
      }

      function formatCompressorTimeout(value) {
        const numeric = Number(value);
        if (!Number.isFinite(numeric)) {
          return '-';
        }
        if (numeric <= 0) {
          return 'Ready';
        }
        if (numeric < 60) {
          return `${numeric.toFixed(1)} s`;
        }
        const minutes = Math.floor(numeric / 60);
        const seconds = Math.round(numeric % 60);
        const minuteLabel = minutes === 1 ? 'min' : 'mins';
        const secondLabel = seconds === 1 ? 'sec' : 'secs';
        if (seconds === 0) {
          return `${minutes} ${minuteLabel}`;
        }
        return `${minutes} ${minuteLabel} ${seconds} ${secondLabel}`;
      }

      function renderCurrentTime() {
        const currentTimeElement = document.getElementById('currentTime');
        if (!currentTimeElement) {
          return;
        }
        let display = '-';
        if (Number.isFinite(currentTimeState.epochSeconds)) {
          const elapsedMs = Date.now() - (currentTimeState.receivedAtMs ?? 0);
          const computedEpoch = currentTimeState.epochSeconds + elapsedMs / 1000;
          display = formatTimestamp(computedEpoch);
        } else if (currentTimeState.formatted) {
          display = currentTimeState.formatted;
        }
        currentTimeElement.textContent = display;
      }

      function updateStatusBar(data) {
        const wifiInfo = document.getElementById('wifiInfo');
        wifiInfo.textContent = data.ssid && data.ip ? `${data.ssid} @ ${data.ip}` : '-';

        const epochValue = Number(data.currentTimeEpoch);
        const timeString =
          typeof data.currentTime === 'string' ? data.currentTime.trim() : '';
        currentTimeState = {
          epochSeconds: Number.isFinite(epochValue) && epochValue > 0 ? epochValue : null,
          receivedAtMs: Date.now(),
          formatted: timeString !== '' ? timeString : null,
        };
        if (currentTimeState.formatted === null && currentTimeState.epochSeconds === null) {
          currentTimeState.receivedAtMs = null;
        }
        renderCurrentTime();

        document.getElementById('systemMode').textContent =
          systemModeLabels[data.systemMode] || data.systemMode || '-';
        document.getElementById('fanMode').textContent =
          fanModeLabels[data.fanMode] || data.fanMode || '-';
        document.getElementById('fanSpeed').textContent =
          fanSpeedLabels[data.fanSpeed] || data.fanSpeed || '-';
        document.getElementById('compressor').textContent = data.compressor ? 'Running' : 'Idle';
        document.getElementById('compressorTimeout').textContent =
          formatCompressorTimeout(data.compressorTimeout);
        document.getElementById('compressorOffTimeout').textContent =
          formatCompressorTimeout(data.compressorOffTimeout);
        document.getElementById('compressorCooldown').textContent =
          data.compressorCooldown ? 'Active' : 'Idle';
        document.getElementById('compressorCooldownRemaining').textContent =
          formatCompressorTimeout(data.compressorCooldownRemaining);
        document.getElementById('target').textContent = toFixedOrDash(data.target);
        document.getElementById('hysteresis').textContent = toFixedOrDash(data.hysteresis);
        document.getElementById('compressorTempLimit').textContent = toFixedOrDash(
          data.compressorTempLimit,
        );
        document.getElementById('compressorMinAmbient').textContent = toFixedOrDash(
          data.compressorMinAmbient,
        );
        document.getElementById('compressorCooldownTemp').textContent = toFixedOrDash(
          data.compressorCooldownTemp,
        );
        document.getElementById('compressorCooldownMinutes').textContent = toFixedOrDash(
          data.compressorCooldownMinutes,
          2,
        );
        document.getElementById('ambient').textContent = toFixedOrDash(data.ambient);
        document.getElementById('coil').textContent = toFixedOrDash(data.coil);
        document.getElementById('energy').textContent = toFixedOrDash(data.energyWh);
      }

      function updateConfigForm(data) {
        document.getElementById('targetInput').value = Number(data.target).toFixed(1);
        document.getElementById('hysteresisInput').value = Number(data.hysteresis).toFixed(1);
        const compressorLimitValue = Number(data.compressorTempLimit);
        document.getElementById('compressorTempLimitInput').value = Number.isFinite(
          compressorLimitValue,
        )
          ? compressorLimitValue.toFixed(1)
          : '';
        const compressorMinAmbientValue = Number(data.compressorMinAmbient);
        document.getElementById('compressorMinAmbientInput').value = Number.isFinite(
          compressorMinAmbientValue,
        )
          ? compressorMinAmbientValue.toFixed(1)
          : '';
        const compressorCooldownTempValue = Number(data.compressorCooldownTemp);
        document.getElementById('compressorCooldownTempInput').value = Number.isFinite(
          compressorCooldownTempValue,
        )
          ? compressorCooldownTempValue.toFixed(1)
          : '';
        const compressorCooldownMinutesValue = Number(data.compressorCooldownMinutes);
        document.getElementById('compressorCooldownMinutesInput').value = Number.isFinite(
          compressorCooldownMinutesValue,
        )
          ? compressorCooldownMinutesValue.toFixed(2)
          : '';
        document.getElementById('fanModeInput').value = data.fanMode;
        document.getElementById('systemModeInput').value = data.systemMode;
        document.getElementById('schedulingInput').checked = Boolean(data.scheduling);
        if (timezoneOffsetInput) {
          const timezoneOffsetValue = Number(data.timezoneOffset);
          const detectedFormatted = formatOffsetHours(detectedTimezoneOffsetHours);
          let displayValue = '';
          if (Number.isFinite(timezoneOffsetValue)) {
            displayValue = formatOffsetHours(timezoneOffsetValue);
          }
          if (!timezoneOffsetAutoApplied) {
            const shouldUseDetected =
              !Number.isFinite(timezoneOffsetValue) || Math.abs(timezoneOffsetValue) < 0.01;
            if (shouldUseDetected && detectedFormatted !== '') {
              displayValue = detectedFormatted;
            }
            timezoneOffsetAutoApplied = true;
          }
          if (displayValue === '' && detectedFormatted !== '') {
            displayValue = detectedFormatted;
          }
          timezoneOffsetInput.value = displayValue;
        }
        document.getElementById('weekdayInput').value = scheduleToText(data.weekday || []);
        document.getElementById('weekendInput').value = scheduleToText(data.weekend || []);
      }

      function updateLogs(data) {
        renderLogs(document.getElementById('temperatureLog'), data.temperatureLog || [], [
          't',
          'ambient',
          'coil',
        ]);
        renderLogs(document.getElementById('powerLog'), data.powerLog || [], [
          't',
          'watts',
          'wh',
          'fan',
          'compressor',
        ]);
      }

      async function refreshState(options = {}) {
        const { updateForm = false } = options;
        const data = await fetchState();
        updateStatusBar(data);
        updateLogs(data);
        if (updateForm) {
          updateConfigForm(data);
        }
      }

      function handleRefreshError(error, showToUser) {
        console.error(error);
        if (showToUser) {
          configStatus.textContent = error.message;
          configStatus.style.color = '#b00020';
        }
      }

      configForm.addEventListener('submit', async (event) => {
        event.preventDefault();
        const formData = new FormData(configForm);
        if (!configForm.scheduling.checked) {
          formData.set('scheduling', 'false');
        }
        const payload = new URLSearchParams(formData);
        configStatus.textContent = 'Saving…';
        configStatus.style.color = '#333';
        try {
          const response = await fetch('/api/config', {
            method: 'POST',
            body: payload,
          });
          if (!response.ok) {
            throw new Error('Failed to save configuration');
          }
          await refreshState({ updateForm: true });
          configStatus.textContent = 'Configuration saved.';
          configStatus.style.color = '#006400';
        } catch (err) {
          configStatus.textContent = err.message;
          configStatus.style.color = '#b00020';
        }
      });

      (async () => {
        try {
          await refreshState({ updateForm: true });
        } catch (err) {
          handleRefreshError(err, true);
        }
      })();

      renderCurrentTime();
      setInterval(renderCurrentTime, 1000);
      setInterval(() => {
        refreshState().catch((err) => handleRefreshError(err, false));
      }, 5000);
    </script>
  </body>
</html>)rawliteral";

}  // namespace interface

