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
      .chart-card-header {
        display: flex;
        flex-wrap: wrap;
        justify-content: space-between;
        align-items: center;
        gap: 0.75rem;
      }
      .power-controls {
        display: flex;
        flex-wrap: wrap;
        align-items: center;
        gap: 0.5rem;
      }
      .power-controls-label {
        font-weight: 600;
        color: #1f2937;
      }
      .power-range-buttons {
        display: inline-flex;
        flex-wrap: wrap;
        gap: 0.5rem;
      }
      .range-button {
        width: auto;
        padding: 0.35rem 0.9rem;
        border-radius: 999px;
        border: 1px solid #0078d4;
        background: rgba(0, 120, 212, 0.08);
        color: #0f3558;
        font-weight: 600;
        transition: background 0.2s ease, color 0.2s ease, border-color 0.2s ease;
      }
      .range-button:hover:not(:disabled) {
        background: rgba(0, 120, 212, 0.2);
      }
      .range-button.active {
        background: #0078d4;
        border-color: #0062aa;
        color: #fff;
      }
      .range-button:disabled {
        background: #e2e8f0;
        border-color: #cbd5e1;
        color: #94a3b8;
      }
      .danger-button {
        width: auto;
        padding: 0.35rem 0.9rem;
        border-radius: 999px;
        border: 1px solid #dc2626;
        background: #dc2626;
        color: #fff;
        font-weight: 600;
        transition: background 0.2s ease, border-color 0.2s ease;
      }
      .danger-button:hover:not(:disabled) {
        background: #b91c1c;
        border-color: #991b1b;
      }
      .danger-button:disabled {
        background: #fca5a5;
        border-color: #f87171;
        color: #7f1d1d;
        cursor: not-allowed;
      }
      .power-summary {
        display: grid;
        grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
        gap: 0.75rem;
      }
      .summary-card {
        background: #f8fafc;
        border: 1px solid #d8e0e7;
        border-radius: 8px;
        padding: 0.75rem;
        display: flex;
        flex-direction: column;
        gap: 0.25rem;
      }
      .summary-label {
        font-size: 0.75rem;
        letter-spacing: 0.06em;
        text-transform: uppercase;
        color: #64748b;
      }
      .summary-value {
        font-size: 1.35rem;
        font-weight: 600;
        color: #0f172a;
      }
      .muted {
        color: #64748b;
        font-size: 0.9rem;
      }
      .power-notice {
        margin: 0.35rem 0 0;
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
      .chart-card {
        display: flex;
        flex-direction: column;
        gap: 0.75rem;
      }
      .chart-card canvas {
        width: 100%;
        max-width: 100%;
      }
      canvas.chart {
        display: block;
        background: #fafbfd;
        border: 1px solid #d8e0e7;
        border-radius: 6px;
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
        <div>Uptime: <span id="uptime">-</span></div>
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
        <div class="chart-card">
          <h3>Temperature (°C)</h3>
          <canvas id="temperatureChart" class="chart" width="720" height="320"></canvas>
        </div>
        <div class="chart-card">
          <div class="chart-card-header">
            <h3>Power (W)</h3>
            <div class="power-controls">
              <span class="power-controls-label">Range</span>
              <div class="power-range-buttons">
                <button type="button" class="range-button active" data-power-range="day">
                  Past day
                </button>
                <button type="button" class="range-button" data-power-range="week">
                  Past week
                </button>
                <button type="button" class="range-button" data-power-range="month">
                  Past month
                </button>
                <button type="button" class="range-button" data-power-range="year">
                  Past year
                </button>
              </div>
              <button type="button" id="resetPowerLogButton" class="danger-button">Reset power log</button>
            </div>
          </div>
          <div class="power-summary">
            <div class="summary-card">
              <div class="summary-label">Total usage</div>
              <div class="summary-value" id="powerSummaryTotal">-</div>
            </div>
            <div class="summary-card">
              <div class="summary-label">Average per day</div>
              <div class="summary-value" id="powerSummaryAverage">-</div>
            </div>
          </div>
          <p id="powerRangeMessage" class="muted power-notice">Loading power history…</p>
          <canvas id="powerChart" class="chart" width="720" height="320"></canvas>
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
      const powerRangeButtons = Array.from(
        document.querySelectorAll('.range-button[data-power-range]'),
      );
      const powerResetButton = document.getElementById('resetPowerLogButton');
      const powerSummaryTotal = document.getElementById('powerSummaryTotal');
      const powerSummaryAverage = document.getElementById('powerSummaryAverage');
      const powerRangeMessage = document.getElementById('powerRangeMessage');
      const powerRangeDurations = {
        day: 24 * 60 * 60 * 1000,
        week: 7 * 24 * 60 * 60 * 1000,
        month: 30 * 24 * 60 * 60 * 1000,
        year: 365 * 24 * 60 * 60 * 1000,
      };
      let selectedPowerRange = 'day';
      let lastPowerHistoryRefresh = 0;
      const powerHistoryRefreshIntervalMs = 60 * 1000;
      let powerHistoryRequest = null;

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

      function renderLineChart(canvas, series, options = {}) {
        if (!canvas) {
          return;
        }
        const context = canvas.getContext('2d');
        if (!context) {
          return;
        }

        const width = canvas.width;
        const height = canvas.height;
        context.clearRect(0, 0, width, height);
        context.fillStyle = '#ffffff';
        context.fillRect(0, 0, width, height);

        const processedSeries = series
          .map((item) => ({
            label: item.label,
            color: item.color,
            data: item.data
              .map((point) => {
                const x = Number(point.x);
                const yValue =
                  point.y === null || typeof point.y === 'undefined'
                    ? NaN
                    : Number(point.y);
                return {
                  x,
                  y: Number.isFinite(yValue) ? yValue : NaN,
                };
              })
              .filter((point) => Number.isFinite(point.x)),
          }))
          .filter((item) => item.data.length > 0);

        const drawEmptyState = (message) => {
          context.fillStyle = '#6b7280';
          context.font = '14px sans-serif';
          context.textAlign = 'center';
          context.textBaseline = 'middle';
          context.fillText(message, width / 2, height / 2);
        };

        if (processedSeries.length === 0) {
          drawEmptyState('No data available');
          return;
        }

        const xValues = processedSeries.flatMap((item) => item.data.map((point) => point.x));
        if (xValues.length === 0) {
          drawEmptyState('No data available');
          return;
        }
        let xMin = Math.min(...xValues);
        let xMax = Math.max(...xValues);
        if (!Number.isFinite(xMin) || !Number.isFinite(xMax)) {
          drawEmptyState('No data available');
          return;
        }
        if (xMax === xMin) {
          xMin -= 30000;
          xMax += 30000;
        }

        const yValues = processedSeries.flatMap((item) =>
          item.data.filter((point) => Number.isFinite(point.y)).map((point) => point.y),
        );
        if (yValues.length === 0) {
          drawEmptyState('No data available');
          return;
        }
        let yMin = Math.min(...yValues);
        let yMax = Math.max(...yValues);
        if (yMax === yMin) {
          const padding = yMin === 0 ? 1 : Math.abs(yMin) * 0.1;
          yMin -= padding;
          yMax += padding;
        }
        const yPadding = (yMax - yMin) * 0.1;
        if (yPadding > 0) {
          yMin -= yPadding;
          yMax += yPadding;
        }

        const margin = options.margin || { left: 60, right: 18, top: 52, bottom: 42 };
        const chartWidth = Math.max(1, width - margin.left - margin.right);
        const chartHeight = Math.max(1, height - margin.top - margin.bottom);

        const xRange = xMax - xMin;
        const yRange = yMax - yMin;
        if (xRange <= 0 || yRange <= 0) {
          drawEmptyState('Waiting for more data');
          return;
        }

        const toX = (value) => margin.left + ((value - xMin) / xRange) * chartWidth;
        const toY = (value) => margin.top + chartHeight - ((value - yMin) / yRange) * chartHeight;

        const xFormatter = typeof options.xFormatter === 'function' ? options.xFormatter : null;

        context.strokeStyle = '#e2e8f0';
        context.lineWidth = 1;
        const xTicks = options.xTicks ?? 5;
        for (let i = 0; i <= xTicks; ++i) {
          const ratio = i / xTicks;
          const x = margin.left + ratio * chartWidth;
          context.beginPath();
          context.moveTo(x, margin.top);
          context.lineTo(x, margin.top + chartHeight);
          context.stroke();
        }
        const yTicks = options.yTicks ?? 5;
        for (let i = 0; i <= yTicks; ++i) {
          const ratio = i / yTicks;
          const y = margin.top + chartHeight - ratio * chartHeight;
          context.beginPath();
          context.moveTo(margin.left, y);
          context.lineTo(margin.left + chartWidth, y);
          context.stroke();
        }

        context.strokeStyle = '#94a3b8';
        context.beginPath();
        context.moveTo(margin.left, margin.top);
        context.lineTo(margin.left, margin.top + chartHeight);
        context.lineTo(margin.left + chartWidth, margin.top + chartHeight);
        context.stroke();

        context.fillStyle = '#475569';
        context.font = '12px sans-serif';
        context.textBaseline = 'top';
        context.textAlign = 'center';
        for (let i = 0; i <= xTicks; ++i) {
          const ratio = i / xTicks;
          const x = margin.left + ratio * chartWidth;
          let label = '';
          if (xFormatter) {
            label = xFormatter({ ratio, range: xRange, min: xMin, max: xMax });
          } else {
            const totalMinutes = xRange / 60000;
            const minuteDecimals =
              totalMinutes >= 60 ? 0 : totalMinutes >= 10 ? 0 : totalMinutes >= 3 ? 1 : 2;
            const minutes = (ratio * xRange) / 60000;
            label = `${minutes.toFixed(minuteDecimals)} min`;
          }
          if (typeof label === 'string' && label.length > 0) {
            context.fillText(label, x, margin.top + chartHeight + 8);
          }
        }

        context.textAlign = 'right';
        context.textBaseline = 'middle';
        const yDecimals = yRange >= 50 ? 0 : yRange >= 10 ? 1 : 2;
        for (let i = 0; i <= yTicks; ++i) {
          const ratio = i / yTicks;
          const y = margin.top + chartHeight - ratio * chartHeight;
          const value = yMin + ratio * yRange;
          const label = value.toFixed(yDecimals);
          if (options.yUnit) {
            context.fillText(`${label} ${options.yUnit}`, margin.left - 8, y);
          } else {
            context.fillText(label, margin.left - 8, y);
          }
        }

        context.textAlign = 'left';
        context.textBaseline = 'middle';
        const legendY = Math.max(18, margin.top - 26);
        const legendSpacing = 132;
        processedSeries.forEach((item, index) => {
          const x = margin.left + index * legendSpacing;
          context.fillStyle = item.color;
          context.fillRect(x, legendY, 12, 12);
          context.fillStyle = '#1f2937';
          context.fillText(item.label, x + 16, legendY + 6);
        });

        processedSeries.forEach((item) => {
          context.beginPath();
          context.strokeStyle = item.color;
          context.lineWidth = 2;
          let started = false;
          const sorted = [...item.data].sort((a, b) => a.x - b.x);
          sorted.forEach((point) => {
            if (!Number.isFinite(point.y)) {
              started = false;
              return;
            }
            const x = toX(point.x);
            const y = toY(point.y);
            if (!started) {
              context.moveTo(x, y);
              started = true;
            } else {
              context.lineTo(x, y);
            }
          });
          context.stroke();
        });
      }

      function renderTemperatureChart(canvas, entries) {
        const prepared = Array.isArray(entries)
          ? entries
              .map((entry) => {
                const time = Number(entry.t);
                if (!Number.isFinite(time)) {
                  return null;
                }
                const ambientValue =
                  entry.ambient === null || typeof entry.ambient === 'undefined'
                    ? NaN
                    : Number(entry.ambient);
                const coilValue =
                  entry.coil === null || typeof entry.coil === 'undefined'
                    ? NaN
                    : Number(entry.coil);
                return {
                  time,
                  ambient: Number.isFinite(ambientValue) ? ambientValue : NaN,
                  coil: Number.isFinite(coilValue) ? coilValue : NaN,
                };
              })
              .filter((entry) => entry !== null)
          : [];

        renderLineChart(
          canvas,
          [
            {
              label: 'Ambient',
              color: '#ef6f6c',
              data: prepared.map((entry) => ({ x: entry.time, y: entry.ambient })),
            },
            {
              label: 'Coil',
              color: '#3f8efc',
              data: prepared.map((entry) => ({ x: entry.time, y: entry.coil })),
            },
          ],
          { yUnit: '°C' },
        );
      }

      function renderPowerChart(canvas, entries) {
        const prepared = Array.isArray(entries)
          ? entries
              .map((entry) => {
                const time = Number(entry.t);
                if (!Number.isFinite(time)) {
                  return null;
                }
                const wattsValue =
                  entry.watts === null || typeof entry.watts === 'undefined'
                    ? NaN
                    : Number(entry.watts);
                return {
                  time,
                  watts: Number.isFinite(wattsValue) ? wattsValue : NaN,
                };
              })
              .filter((entry) => entry !== null)
          : [];

        const xFormatter = ({ ratio, range }) => {
          if (!Number.isFinite(range) || range <= 0) {
            return '';
          }
          const totalHours = range / 3600000;
          if (totalHours >= 24 * 30) {
            const totalDays = range / 86400000;
            const days = ratio * totalDays;
            const decimals = totalDays >= 60 ? 0 : totalDays >= 14 ? 1 : 2;
            return `${days.toFixed(decimals)} d`;
          }
          if (totalHours >= 24) {
            const totalDays = range / 86400000;
            const days = ratio * totalDays;
            const decimals = totalDays >= 10 ? 1 : 2;
            return `${days.toFixed(decimals)} d`;
          }
          if (totalHours >= 6) {
            const hours = ratio * totalHours;
            return `${hours.toFixed(0)} h`;
          }
          const minutes = (ratio * range) / 60000;
          const decimals = minutes >= 10 ? 0 : 1;
          return `${minutes.toFixed(decimals)} min`;
        };

        renderLineChart(
          canvas,
          [
            {
              label: 'Watts',
              color: '#2ca02c',
              data: prepared.map((entry) => ({ x: entry.time, y: entry.watts })),
            },
          ],
          { yUnit: 'W', xFormatter },
        );
      }

      function setActivePowerRange(range) {
        powerRangeButtons.forEach((button) => {
          if (button.dataset.powerRange === range) {
            button.classList.add('active');
          } else {
            button.classList.remove('active');
          }
        });
      }

      function formatEnergyWh(value) {
        const numeric = Number(value);
        if (!Number.isFinite(numeric)) {
          return '-';
        }
        const absolute = Math.abs(numeric);
        if (absolute >= 1000) {
          const kilo = numeric / 1000;
          const decimals = Math.abs(kilo) >= 10 ? 1 : 2;
          return `${kilo.toFixed(decimals)} kWh`;
        }
        if (absolute >= 100) {
          return `${numeric.toFixed(0)} Wh`;
        }
        return `${numeric.toFixed(1)} Wh`;
      }

      function formatShortDate(timestampMs) {
        const numeric = Number(timestampMs);
        if (!Number.isFinite(numeric)) {
          return '';
        }
        const date = new Date(numeric);
        const month = date.toLocaleDateString(undefined, { month: 'short' });
        const day = String(date.getDate()).padStart(2, '0');
        const hours = String(date.getHours()).padStart(2, '0');
        const minutes = String(date.getMinutes()).padStart(2, '0');
        return `${month} ${day} ${hours}:${minutes}`;
      }

      function describeDuration(ms) {
        const numeric = Number(ms);
        if (!Number.isFinite(numeric) || numeric <= 0) {
          return '0 minutes';
        }
        const days = numeric / 86400000;
        if (days >= 1) {
          const decimals = days >= 10 ? 0 : 1;
          const value = days.toFixed(decimals);
          return `${value} day${Number(value) === 1 ? '' : 's'}`;
        }
        const hours = numeric / 3600000;
        if (hours >= 1) {
          const decimals = hours >= 10 ? 0 : 1;
          const value = hours.toFixed(decimals);
          return `${value} hour${Number(value) === 1 ? '' : 's'}`;
        }
        const minutes = numeric / 60000;
        const decimals = minutes >= 10 ? 0 : 1;
        const value = minutes.toFixed(decimals);
        return `${value} minute${Number(value) === 1 ? '' : 's'}`;
      }

      function updatePowerRangeAvailability(meta) {
        const availableCount = Number(meta?.availableCount) || 0;
        const availableSpanMs = Number(meta?.availableSpanMs);
        let fallbackRange = null;
        powerRangeButtons.forEach((button) => {
          const range = button.dataset.powerRange;
          const duration = powerRangeDurations[range] || powerRangeDurations.week;
          let enabled = availableCount >= 2;
          if (range !== 'day' && range !== 'week') {
            enabled = enabled && Number.isFinite(availableSpanMs) && availableSpanMs >= duration;
          }
          button.disabled = !enabled;
          if (enabled && fallbackRange === null) {
            fallbackRange = range;
          }
        });
        const selectedButton = powerRangeButtons.find(
          (button) => button.dataset.powerRange === selectedPowerRange,
        );
        if (selectedButton && selectedButton.disabled && fallbackRange && fallbackRange !== selectedPowerRange) {
          selectedPowerRange = fallbackRange;
          setActivePowerRange(selectedPowerRange);
          return true;
        }
        return false;
      }

      function applyPowerHistoryResponse(data) {
        const entries = Array.isArray(data?.entries) ? data.entries : [];
        renderPowerChart(document.getElementById('powerChart'), entries);

        const baselineWh = Number(data?.rangeStartEnergyWh);
        const endWh = Number(data?.rangeEndEnergyWh);
        let totalWh =
          Number.isFinite(baselineWh) && Number.isFinite(endWh) ? endWh - baselineWh : NaN;
        if (Number.isFinite(totalWh) && totalWh < 0) {
          totalWh = 0;
        }
        const filteredSpanMs = Number(data?.filteredSpanMs);
        const filteredCount = Number(data?.filteredCount) || entries.length;

        if (!Number.isFinite(totalWh) || filteredCount < 2) {
          powerSummaryTotal.textContent = '-';
          powerSummaryAverage.textContent = '-';
        } else {
          powerSummaryTotal.textContent = formatEnergyWh(totalWh);
          const days = filteredSpanMs > 0 ? filteredSpanMs / (24 * 60 * 60 * 1000) : 0;
          if (days > 0.05) {
            powerSummaryAverage.textContent = `${formatEnergyWh(totalWh / days)} / day`;
          } else {
            powerSummaryAverage.textContent = `${formatEnergyWh(totalWh)} / day`;
          }
        }

        if (!powerRangeMessage) {
          return;
        }

        if (filteredCount < 2) {
          powerRangeMessage.textContent =
            'Collecting power history… need at least two samples to chart this range.';
          return;
        }

        const rangeDuration = powerRangeDurations[selectedPowerRange] || powerRangeDurations.week;
        const coverageMs = Number.isFinite(filteredSpanMs) ? filteredSpanMs : 0;
        if (entries.length === 0) {
          powerRangeMessage.textContent = 'No power history available yet.';
          return;
        }

        const coverageAdequate = Number.isFinite(rangeDuration)
          ? coverageMs + 60000 >= rangeDuration
          : true;
        if (!coverageAdequate) {
          const remainingMs = Math.max(0, rangeDuration - coverageMs);
          const neededDays = Math.max(0, Math.ceil(remainingMs / (24 * 60 * 60 * 1000)));
          const coverageLabel = describeDuration(coverageMs);
          const suffix = neededDays > 0 ? `${neededDays} more day${neededDays === 1 ? '' : 's'}` : 'more time';
          powerRangeMessage.textContent = `Only ${coverageLabel} of history so far – ${suffix} needed for a full ${selectedPowerRange}.`;
          return;
        }

        const startText = formatShortDate(entries[0].t);
        const endText = formatShortDate(entries[entries.length - 1].t);
        if (startText && endText) {
          powerRangeMessage.textContent = `Showing ${filteredCount} samples from ${startText} to ${endText}.`;
        } else {
          powerRangeMessage.textContent = `Showing ${filteredCount} samples spanning ${describeDuration(
            coverageMs,
          )}.`;
        }
      }

      async function refreshPowerHistory(force = false) {
        if (powerRangeButtons.length === 0) {
          return;
        }
        if (powerHistoryRequest) {
          if (!force) {
            return powerHistoryRequest;
          }
          try {
            await powerHistoryRequest;
          } catch (err) {
            console.error(err);
          }
        }

        const now = Date.now();
        if (!force && now - lastPowerHistoryRefresh < powerHistoryRefreshIntervalMs) {
          return;
        }
        lastPowerHistoryRefresh = now;

        const requestPromise = (async () => {
          let referenceMs = Date.now();
          if (
            currentTimeState.epochSeconds !== null &&
            currentTimeState.receivedAtMs !== null
          ) {
            referenceMs =
              currentTimeState.epochSeconds * 1000 + (Date.now() - currentTimeState.receivedAtMs);
          } else if (currentTimeState.epochSeconds !== null) {
            referenceMs = currentTimeState.epochSeconds * 1000;
          }
          const duration = powerRangeDurations[selectedPowerRange] || powerRangeDurations.week;
          const startMs = Math.max(0, Math.floor(referenceMs - duration));
          const params = new URLSearchParams();
          if (Number.isFinite(startMs)) {
            params.set('start', String(startMs));
          }
          const url = params.toString().length > 0 ? `/api/power-log?${params}` : '/api/power-log';
          if (powerRangeMessage) {
            powerRangeMessage.textContent = 'Loading power history…';
          }
          const response = await fetch(url);
          if (!response.ok) {
            throw new Error('Failed to load power history');
          }
          const data = await response.json();
          const switched = updatePowerRangeAvailability(data);
          if (switched) {
            lastPowerHistoryRefresh = 0;
            setTimeout(() => {
              refreshPowerHistory(true).catch((err) => console.error(err));
            }, 0);
            return;
          }
          applyPowerHistoryResponse(data);
        })();

        powerHistoryRequest = requestPromise;
        try {
          await requestPromise;
        } catch (error) {
          console.error(error);
          if (powerRangeMessage) {
            powerRangeMessage.textContent = error.message || 'Power history unavailable.';
          }
          powerSummaryTotal.textContent = '-';
          powerSummaryAverage.textContent = '-';
          renderPowerChart(document.getElementById('powerChart'), []);
        } finally {
          powerHistoryRequest = null;
        }
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

      function formatUptimeSeconds(value) {
        const numeric = Number(value);
        if (!Number.isFinite(numeric) || numeric < 0) {
          return '-';
        }
        const totalSeconds = Math.floor(numeric);
        const days = Math.floor(totalSeconds / 86400);
        const hours = Math.floor((totalSeconds % 86400) / 3600);
        const minutes = Math.floor((totalSeconds % 3600) / 60);
        const seconds = totalSeconds % 60;
        const parts = [];
        if (days > 0) {
          parts.push(`${days}d`);
        }
        if (hours > 0 || parts.length > 0) {
          parts.push(`${hours}h`);
        }
        if (minutes > 0 || parts.length > 0) {
          parts.push(`${minutes}m`);
        }
        if (parts.length === 0 || seconds > 0) {
          parts.push(`${seconds}s`);
        }
        return parts.join(' ');
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
        const uptimeElement = document.getElementById('uptime');
        if (uptimeElement) {
          uptimeElement.textContent = formatUptimeSeconds(data.uptimeSeconds);
        }
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
        renderTemperatureChart(
          document.getElementById('temperatureChart'),
          data && Array.isArray(data.temperatureLog) ? data.temperatureLog : [],
        );
      }

      async function refreshState(options = {}) {
        const { updateForm = false } = options;
        const data = await fetchState();
        updateStatusBar(data);
        updateLogs(data);
        if (updateForm) {
          updateConfigForm(data);
        }
        refreshPowerHistory().catch((err) => console.error(err));
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

      setActivePowerRange(selectedPowerRange);
      powerRangeButtons.forEach((button) => {
        button.addEventListener('click', () => {
          if (button.disabled) {
            return;
          }
          const range = button.dataset.powerRange;
          if (range && range !== selectedPowerRange) {
            selectedPowerRange = range;
            setActivePowerRange(selectedPowerRange);
            refreshPowerHistory(true).catch((err) => console.error(err));
          }
        });
      });

      if (powerResetButton) {
        powerResetButton.addEventListener('click', async () => {
          const confirmed = window.confirm(
            'Reset power history? This will remove all recorded power log entries.',
          );
          if (!confirmed) {
            return;
          }

          const originalLabel = powerResetButton.textContent.trim() || 'Reset power log';
          powerResetButton.disabled = true;
          powerResetButton.textContent = 'Resetting…';

          try {
            const response = await fetch('/api/power-log', { method: 'DELETE' });
            if (!response.ok) {
              throw new Error('Failed to reset power log');
            }

            if (powerRangeMessage) {
              powerRangeMessage.textContent = 'Power history cleared.';
            }
            powerSummaryTotal.textContent = '-';
            powerSummaryAverage.textContent = '-';
            renderPowerChart(document.getElementById('powerChart'), []);
            lastPowerHistoryRefresh = 0;
            await refreshPowerHistory(true);
          } catch (err) {
            console.error(err);
            const message = err && err.message ? err.message : 'Failed to reset power log.';
            if (powerRangeMessage) {
              powerRangeMessage.textContent = message;
            }
            window.alert(message);
          } finally {
            powerResetButton.disabled = false;
            powerResetButton.textContent = originalLabel;
          }
        });
      }

      (async () => {
        try {
          await refreshState({ updateForm: true });
          await refreshPowerHistory(true);
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

