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
                <button type="button" class="range-button active" data-power-range="current">
                  Current
                </button>
                <button type="button" class="range-button" data-power-range="day">
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
        uptimeMs: null,
        uptimeReceivedAtMs: null,
        clockOffsetMs: null,
      };
      const powerRangeButtons = Array.from(
        document.querySelectorAll('.range-button[data-power-range]'),
      );
      const powerResetButton = document.getElementById('resetPowerLogButton');
      const powerSummaryTotal = document.getElementById('powerSummaryTotal');
      const powerSummaryAverage = document.getElementById('powerSummaryAverage');
      const powerRangeMessage = document.getElementById('powerRangeMessage');
      const currentPowerWindowMs = 30 * 60 * 1000;
      const powerRangeDurations = {
        current: currentPowerWindowMs,
        day: 24 * 60 * 60 * 1000,
        week: 7 * 24 * 60 * 60 * 1000,
        month: 30 * 24 * 60 * 60 * 1000,
        year: 365 * 24 * 60 * 60 * 1000,
      };
      let selectedPowerRange = 'current';
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

      function renderCurrentPowerChart(canvas, entries) {
        const summary = {
          type: 'current',
          sampleCount: 0,
          hasData: false,
          windowStartEpoch: null,
          windowEndEpoch: null,
        };
        if (!canvas) {
          return summary;
        }
        const context = canvas.getContext('2d');
        if (!context) {
          return summary;
        }

        const drawEmptyState = (message) => {
          context.clearRect(0, 0, canvas.width, canvas.height);
          context.fillStyle = '#ffffff';
          context.fillRect(0, 0, canvas.width, canvas.height);
          context.fillStyle = '#6b7280';
          context.font = '14px sans-serif';
          context.textAlign = 'center';
          context.textBaseline = 'middle';
          context.fillText(message, canvas.width / 2, canvas.height / 2);
        };

        const prepared = Array.isArray(entries)
          ? entries
              .map((entry) => {
                const uptime = Number(entry?.t);
                if (!Number.isFinite(uptime)) {
                  return null;
                }
                const wattsValue =
                  entry.watts === null || typeof entry.watts === 'undefined'
                    ? NaN
                    : Number(entry.watts);
                if (!Number.isFinite(wattsValue)) {
                  return null;
                }
                const epochValue = uptimeToEpochMs(uptime);
                const axisTime = Number.isFinite(epochValue) ? epochValue : uptime;
                return {
                  axisTime,
                  watts: wattsValue,
                };
              })
              .filter((entry) => entry !== null)
          : [];

        if (prepared.length === 0) {
          drawEmptyState('No power samples yet.');
          return summary;
        }

        const sorted = [...prepared].sort((a, b) => a.axisTime - b.axisTime);
        const latestEntry = sorted[sorted.length - 1];
        const latestTime = latestEntry?.axisTime;
        if (!Number.isFinite(latestTime)) {
          drawEmptyState('No recent power samples.');
          return summary;
        }

        const earliestTime = sorted[0]?.axisTime ?? latestTime;
        const windowStart = Math.max(latestTime - currentPowerWindowMs, earliestTime);
        const recent = sorted.filter((entry) => entry.axisTime >= windowStart);
        if (recent.length === 0) {
          drawEmptyState('No power samples in the last 30 minutes.');
          return summary;
        }

        summary.sampleCount = recent.length;
        summary.hasData = true;
        summary.windowStartEpoch = windowStart;
        summary.windowEndEpoch = latestTime;

        const chartData = [...recent];
        if (chartData.length > 0 && chartData[0].axisTime > windowStart) {
          chartData.unshift({ axisTime: windowStart, watts: chartData[0].watts });
        }
        if (chartData.length > 0 && chartData[chartData.length - 1].axisTime < latestTime) {
          chartData.push({ axisTime: latestTime, watts: chartData[chartData.length - 1].watts });
        }

        const xFormatter = ({ ratio, range }) => {
          if (!Number.isFinite(range) || range <= 0) {
            return '';
          }
          const minutes = (ratio * range) / 60000;
          const decimals = range >= 20 * 60000 ? 0 : range >= 5 * 60000 ? 1 : 2;
          return `${minutes.toFixed(decimals)} min`;
        };

        renderLineChart(
          canvas,
          [
            {
              label: 'Watts',
              color: '#2ca02c',
              data: chartData.map((entry) => ({ x: entry.axisTime, y: entry.watts })),
            },
          ],
          {
            yUnit: 'W',
            xFormatter,
            margin: { left: 60, right: 18, top: 48, bottom: 48 },
          },
        );

        return summary;
      }

      function renderPowerChart(canvas, entries, meta = {}) {
        if (selectedPowerRange === 'current') {
          return renderCurrentPowerChart(canvas, entries);
        }
        if (selectedPowerRange === 'day') {
          return renderDayPowerBarChart(canvas, entries);
        }
        if (selectedPowerRange === 'week') {
          return renderRecentDailyUsageChart(canvas, entries, meta, {
            days: 7,
            title: 'Daily Energy (Last 7 Days)',
          });
        }
        if (selectedPowerRange === 'month') {
          return renderRecentDailyUsageChart(canvas, entries, meta, {
            days: 30,
            title: 'Daily Energy (Last 30 Days)',
          });
        }
        if (selectedPowerRange === 'year') {
          return renderRecentMonthlyUsageChart(canvas, entries, meta, {
            months: 12,
            title: 'Monthly Energy (Last 12 Months)',
          });
        }

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

        return {
          type: 'line',
          hasData: prepared.length > 0,
          sampleCount: prepared.length,
        };
      }

      function renderDayPowerBarChart(canvas, entries) {
        const summary = {
          type: 'day',
          sampleCount: Array.isArray(entries) ? entries.length : 0,
          hasData: false,
          buckets: [],
          coveredBuckets: 0,
          dayStartEpoch: null,
        };
        if (!canvas) {
          return summary;
        }
        const context = canvas.getContext('2d');
        if (!context) {
          return summary;
        }

        const width = canvas.width;
        const height = canvas.height;
        context.clearRect(0, 0, width, height);
        context.fillStyle = '#ffffff';
        context.fillRect(0, 0, width, height);

        const drawEmptyState = (message) => {
          context.fillStyle = '#6b7280';
          context.font = '14px sans-serif';
          context.textAlign = 'center';
          context.textBaseline = 'middle';
          context.fillText(message, width / 2, height / 2);
        };

        let latestSampleUptime = null;
        if (Array.isArray(entries)) {
          entries.forEach((entry) => {
            const time = Number(entry.t);
            if (!Number.isFinite(time)) {
              return;
            }
            if (!Number.isFinite(latestSampleUptime) || time > latestSampleUptime) {
              latestSampleUptime = time;
            }
          });
        }

        const referenceEpochMs = estimateReferenceEpochMs(latestSampleUptime);
        const referenceDate = new Date(referenceEpochMs);
        referenceDate.setHours(0, 0, 0, 0);
        const dayStartEpoch = referenceDate.getTime();
        const dayEndEpoch = dayStartEpoch + 24 * 60 * 60 * 1000;
        summary.dayStartEpoch = dayStartEpoch;

        const buckets = Array.from({ length: 24 }, () => ({ sum: 0, count: 0 }));
        if (Array.isArray(entries)) {
          entries.forEach((entry) => {
            const time = Number(entry.t);
            const wattsValue =
              entry.watts === null || typeof entry.watts === 'undefined'
                ? NaN
                : Number(entry.watts);
            if (!Number.isFinite(time) || !Number.isFinite(wattsValue)) {
              return;
            }
            const epochMs = uptimeToEpochMs(time);
            if (!Number.isFinite(epochMs)) {
              return;
            }
            if (epochMs < dayStartEpoch || epochMs >= dayEndEpoch) {
              return;
            }
            const hourIndex = Math.floor((epochMs - dayStartEpoch) / 3600000);
            if (hourIndex < 0 || hourIndex >= buckets.length) {
              return;
            }
            buckets[hourIndex].sum += wattsValue;
            buckets[hourIndex].count += 1;
          });
        }

        const hasSamples = buckets.some((bucket) => bucket.count > 0);
        summary.hasData = hasSamples;
        summary.coveredBuckets = buckets.filter((bucket) => bucket.count > 0).length;
        summary.buckets = buckets.map((bucket, index) => ({
          start: dayStartEpoch + index * 3600000,
          end: dayStartEpoch + (index + 1) * 3600000,
          averageWatts: bucket.count > 0 ? bucket.sum / bucket.count : 0,
          count: bucket.count,
        }));
        if (!hasSamples) {
          drawEmptyState('No power samples for this day yet');
          return summary;
        }

        const averages = buckets.map((bucket) =>
          bucket.count > 0 ? bucket.sum / bucket.count : 0,
        );
        const maxAverage = Math.max(...averages);
        const yMax = maxAverage > 0 ? Math.max(maxAverage * 1.1, maxAverage + 5) : 10;
        const yTicks = 5;
        const margin = { left: 60, right: 18, top: 48, bottom: 56 };
        const chartWidth = Math.max(1, width - margin.left - margin.right);
        const chartHeight = Math.max(1, height - margin.top - margin.bottom);

        const toY = (value) =>
          margin.top + chartHeight - Math.min(1, Math.max(0, value / yMax)) * chartHeight;

        context.strokeStyle = '#e2e8f0';
        context.lineWidth = 1;
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

        const slotWidth = chartWidth / buckets.length;
        const barWidth = Math.max(6, slotWidth * 0.6);
        const barColor = '#2ca02c';
        const labelDates = Array.from({ length: buckets.length }, (_, index) =>
          new Date(dayStartEpoch + index * 3600000),
        );

        context.fillStyle = barColor;
        buckets.forEach((bucket, index) => {
          const average = averages[index];
          const barHeight = chartHeight * (Math.min(average, yMax) / yMax);
          const x = margin.left + index * slotWidth + (slotWidth - barWidth) / 2;
          const y = margin.top + chartHeight - barHeight;
          context.fillRect(x, y, barWidth, barHeight);
        });

        context.fillStyle = '#475569';
        context.font = '12px sans-serif';
        context.textAlign = 'right';
        context.textBaseline = 'middle';
        for (let i = 0; i <= yTicks; ++i) {
          const ratio = i / yTicks;
          const value = yMax * ratio;
          const y = toY(value);
          const label = value.toFixed(value >= 50 ? 0 : value >= 10 ? 1 : 2);
          context.fillText(`${label} W`, margin.left - 8, y);
        }

        context.textAlign = 'center';
        context.textBaseline = 'top';
        labelDates.forEach((date, index) => {
          const label = date.toLocaleTimeString([], { hour: '2-digit' });
          const x = margin.left + index * slotWidth + slotWidth / 2;
          const y = margin.top + chartHeight + 8;
          if (index % 2 === 0) {
            context.fillText(label, x, y);
          }
        });

        context.textAlign = 'left';
        context.textBaseline = 'middle';
        context.fillStyle = barColor;
        context.fillRect(margin.left, Math.max(18, margin.top - 26), 12, 12);
        context.fillStyle = '#1f2937';
        context.fillText(
          'Average Watts per Hour',
          margin.left + 16,
          Math.max(18, margin.top - 20),
        );

        return summary;
      }

      function renderRecentDailyUsageChart(canvas, entries, meta, options = {}) {
        const dayMs = 24 * 60 * 60 * 1000;
        const days = Number(options.days) > 0 ? Math.floor(Number(options.days)) : 7;
        const summary = {
          type: 'daily-usage',
          sampleCount: Array.isArray(entries) ? entries.length : 0,
          bucketCount: days,
          buckets: [],
          coveredBuckets: 0,
          totalWh: 0,
          windowStartEpoch: null,
          windowEndEpoch: null,
          hasData: false,
        };
        if (!canvas) {
          return summary;
        }
        const context = canvas.getContext('2d');
        if (!context || days <= 0) {
          return summary;
        }

        const latestEntry = Array.isArray(entries) && entries.length > 0 ? entries[entries.length - 1] : null;
        let latestEpoch = latestEntry ? uptimeToEpochMs(latestEntry.t) : null;
        if (!Number.isFinite(latestEpoch)) {
          latestEpoch = estimateReferenceEpochMs();
        }
        const anchorDate = new Date(Number.isFinite(latestEpoch) ? latestEpoch : Date.now());
        anchorDate.setHours(0, 0, 0, 0);
        const windowEnd = anchorDate.getTime() + dayMs;
        const windowStart = windowEnd - days * dayMs;
        summary.windowStartEpoch = windowStart;
        summary.windowEndEpoch = windowEnd;

        const buckets = Array.from({ length: days }, (_, index) => {
          const start = windowStart + index * dayMs;
          return { start, end: start + dayMs, totalWh: 0, samples: 0 };
        });

        let previousWh = Number(meta?.rangeStartEnergyWh);
        if (!Number.isFinite(previousWh)) {
          previousWh = null;
        }

        if (Array.isArray(entries)) {
          entries.forEach((entry) => {
            const whValue = Number(entry?.wh);
            const epoch = uptimeToEpochMs(entry?.t);
            if (!Number.isFinite(whValue)) {
              return;
            }
            if (!Number.isFinite(epoch)) {
              previousWh = whValue;
              return;
            }
            const sampleBucketIndex = Math.floor((epoch - windowStart) / dayMs);
            if (sampleBucketIndex >= 0 && sampleBucketIndex < buckets.length) {
              buckets[sampleBucketIndex].samples =
                (buckets[sampleBucketIndex].samples || 0) + 1;
            }
            if (!Number.isFinite(previousWh)) {
              previousWh = whValue;
              return;
            }
            const delta = whValue - previousWh;
            previousWh = whValue;
            if (!Number.isFinite(delta) || delta <= 0) {
              return;
            }
            if (sampleBucketIndex < 0 || sampleBucketIndex >= buckets.length) {
              return;
            }
            buckets[sampleBucketIndex].totalWh += delta;
          });
        }

        const values = buckets.map((bucket) => (Number.isFinite(bucket.totalWh) ? Math.max(0, bucket.totalWh) : 0));
        summary.buckets = buckets;
        summary.coveredBuckets = buckets.filter((bucket) => (bucket.samples || 0) > 0).length;
        summary.totalWh = values.reduce((sum, value) => sum + value, 0);
        summary.hasData = summary.coveredBuckets > 0;

        renderUsageBarChart(context, buckets, {
          title: options.title,
          emptyMessage: 'No power samples for this period yet',
          hasSamples: summary.coveredBuckets > 0,
          labelFormatter: (bucket, index, count) => {
            const date = new Date(bucket.start);
            if (!summary.hasData) {
              return date.toLocaleDateString(undefined, { month: 'short', day: '2-digit' });
            }
            if (count > 14) {
              const interval = Math.max(1, Math.ceil(count / 10));
              if (index % interval !== 0 && index !== count - 1) {
                return '';
              }
            }
            return date.toLocaleDateString(undefined, { month: 'short', day: '2-digit' });
          },
        });

        return summary;
      }

      function renderRecentMonthlyUsageChart(canvas, entries, meta, options = {}) {
        const months = Number(options.months) > 0 ? Math.floor(Number(options.months)) : 12;
        const summary = {
          type: 'monthly-usage',
          sampleCount: Array.isArray(entries) ? entries.length : 0,
          bucketCount: months,
          buckets: [],
          coveredBuckets: 0,
          totalWh: 0,
          windowStartEpoch: null,
          windowEndEpoch: null,
          hasData: false,
        };
        if (!canvas) {
          return summary;
        }
        const context = canvas.getContext('2d');
        if (!context || months <= 0) {
          return summary;
        }

        const latestEntry = Array.isArray(entries) && entries.length > 0 ? entries[entries.length - 1] : null;
        let latestEpoch = latestEntry ? uptimeToEpochMs(latestEntry.t) : null;
        if (!Number.isFinite(latestEpoch)) {
          latestEpoch = estimateReferenceEpochMs();
        }
        const anchorDate = new Date(Number.isFinite(latestEpoch) ? latestEpoch : Date.now());
        const endMonth = new Date(anchorDate.getFullYear(), anchorDate.getMonth() + 1, 1);
        const startMonth = new Date(endMonth.getFullYear(), endMonth.getMonth() - months, 1);
        summary.windowStartEpoch = startMonth.getTime();
        summary.windowEndEpoch = endMonth.getTime();

        const buckets = Array.from({ length: months }, (_, index) => {
          const startDate = new Date(startMonth.getFullYear(), startMonth.getMonth() + index, 1);
          const endDate = new Date(startMonth.getFullYear(), startMonth.getMonth() + index + 1, 1);
          return { start: startDate.getTime(), end: endDate.getTime(), totalWh: 0, samples: 0 };
        });

        let previousWh = Number(meta?.rangeStartEnergyWh);
        if (!Number.isFinite(previousWh)) {
          previousWh = null;
        }

        if (Array.isArray(entries)) {
          entries.forEach((entry) => {
            const whValue = Number(entry?.wh);
            const epoch = uptimeToEpochMs(entry?.t);
            if (!Number.isFinite(whValue)) {
              return;
            }
            if (!Number.isFinite(epoch)) {
              previousWh = whValue;
              return;
            }
            const bucketIndex = buckets.findIndex(
              (bucket) => epoch >= bucket.start && epoch < bucket.end,
            );
            if (bucketIndex === -1) {
              previousWh = whValue;
              return;
            }
            buckets[bucketIndex].samples = (buckets[bucketIndex].samples || 0) + 1;
            if (!Number.isFinite(previousWh)) {
              previousWh = whValue;
              return;
            }
            const delta = whValue - previousWh;
            previousWh = whValue;
            if (!Number.isFinite(delta) || delta <= 0) {
              return;
            }
            buckets[bucketIndex].totalWh += delta;
          });
        }

        const values = buckets.map((bucket) => (Number.isFinite(bucket.totalWh) ? Math.max(0, bucket.totalWh) : 0));
        summary.buckets = buckets;
        summary.coveredBuckets = buckets.filter((bucket) => (bucket.samples || 0) > 0).length;
        summary.totalWh = values.reduce((sum, value) => sum + value, 0);
        summary.hasData = summary.coveredBuckets > 0;

        renderUsageBarChart(context, buckets, {
          title: options.title,
          emptyMessage: 'No power samples for this period yet',
          hasSamples: summary.coveredBuckets > 0,
          labelFormatter: (bucket, index) => {
            const date = new Date(bucket.start);
            const formatOptions =
              index === 0 || date.getMonth() === 0
                ? { month: 'short', year: 'numeric' }
                : { month: 'short' };
            return date.toLocaleDateString(undefined, formatOptions);
          },
        });

        return summary;
      }

      function renderUsageBarChart(context, buckets, options = {}) {
        const canvas = context.canvas;
        const width = canvas.width;
        const height = canvas.height;
        context.clearRect(0, 0, width, height);
        context.fillStyle = '#ffffff';
        context.fillRect(0, 0, width, height);

        const drawEmptyState = (message) => {
          context.fillStyle = '#6b7280';
          context.font = '14px sans-serif';
          context.textAlign = 'center';
          context.textBaseline = 'middle';
          context.fillText(message, width / 2, height / 2);
        };

        if (!Array.isArray(buckets) || buckets.length === 0) {
          drawEmptyState(options.emptyMessage || 'No data available');
          return;
        }

        const values = buckets.map((bucket) => (Number.isFinite(bucket.totalWh) ? Math.max(0, bucket.totalWh) : 0));
        const hasUsage = values.some((value) => value > 0);
        const hasSamples = Boolean(options.hasSamples);
        if (!hasUsage && !hasSamples) {
          drawEmptyState(options.emptyMessage || 'No data available');
          return;
        }

        const maxValue = Math.max(...values);
        const yMax = maxValue > 0 ? maxValue * 1.1 : 10;
        const margin = { left: 72, right: 18, top: 48, bottom: 64 };
        const chartWidth = Math.max(1, width - margin.left - margin.right);
        const chartHeight = Math.max(1, height - margin.top - margin.bottom);
        const slotWidth = chartWidth / buckets.length;
        const barWidth = Math.max(6, slotWidth * 0.6);
        const useKilowattHours = yMax >= 1000;
        const scale = useKilowattHours ? 1 / 1000 : 1;
        const unitLabel = useKilowattHours ? 'kWh' : 'Wh';
        const yTicks = 5;

        const toY = (value) =>
          margin.top + chartHeight - Math.min(1, Math.max(0, value / yMax)) * chartHeight;

        context.strokeStyle = '#e2e8f0';
        context.lineWidth = 1;
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

        context.fillStyle = '#2563eb';
        buckets.forEach((bucket, index) => {
          const value = values[index];
          const barHeight = chartHeight * (Math.min(value, yMax) / yMax);
          const x = margin.left + index * slotWidth + (slotWidth - barWidth) / 2;
          const y = margin.top + chartHeight - barHeight;
          context.fillRect(x, y, barWidth, barHeight);
        });

        context.fillStyle = '#475569';
        context.font = '12px sans-serif';
        context.textAlign = 'right';
        context.textBaseline = 'middle';
        for (let i = 0; i <= yTicks; ++i) {
          const ratio = i / yTicks;
          const value = yMax * ratio;
          const y = toY(value);
          const labelValue = value * scale;
          const label = labelValue >= 10 ? labelValue.toFixed(0) : labelValue.toFixed(1);
          context.fillText(`${label} ${unitLabel}`, margin.left - 8, y);
        }

        const labelFormatter = typeof options.labelFormatter === 'function'
          ? options.labelFormatter
          : () => '';
        const labelInterval = Math.max(1, Math.ceil(buckets.length / 10));
        context.textAlign = 'center';
        context.textBaseline = 'top';
        buckets.forEach((bucket, index) => {
          const label = labelFormatter(bucket, index, buckets.length);
          if (!label) {
            return;
          }
          if (buckets.length > 12 && index % labelInterval !== 0 && index !== buckets.length - 1) {
            return;
          }
          const x = margin.left + index * slotWidth + slotWidth / 2;
          const y = margin.top + chartHeight + 8;
          context.fillText(label, x, y);
        });

        if (options.title) {
          context.textAlign = 'left';
          context.textBaseline = 'middle';
          context.fillStyle = '#2563eb';
          context.fillRect(margin.left, Math.max(18, margin.top - 26), 12, 12);
          context.fillStyle = '#1f2937';
          context.fillText(options.title, margin.left + 16, Math.max(18, margin.top - 20));
        }
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
        const epochValue = uptimeToEpochMs(numeric);
        const date = new Date(Number.isFinite(epochValue) ? epochValue : numeric);
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

      function estimateCurrentUptimeMs() {
        if (!Number.isFinite(currentTimeState.uptimeMs)) {
          return null;
        }
        if (!Number.isFinite(currentTimeState.uptimeReceivedAtMs)) {
          return currentTimeState.uptimeMs;
        }
        const elapsed = Date.now() - currentTimeState.uptimeReceivedAtMs;
        if (!Number.isFinite(elapsed) || elapsed < 0) {
          return currentTimeState.uptimeMs;
        }
        return currentTimeState.uptimeMs + elapsed;
      }

      function uptimeToEpochMs(uptimeMs) {
        const numeric = Number(uptimeMs);
        if (!Number.isFinite(numeric)) {
          return null;
        }
        if (Number.isFinite(currentTimeState.clockOffsetMs)) {
          return numeric + currentTimeState.clockOffsetMs;
        }
        if (Number.isFinite(currentTimeState.epochSeconds)) {
          const baseEpoch = currentTimeState.epochSeconds * 1000;
          const receivedAt = Number.isFinite(currentTimeState.receivedAtMs)
            ? currentTimeState.receivedAtMs
            : null;
          const elapsedSince = receivedAt !== null ? Date.now() - receivedAt : 0;
          const estimatedCurrentEpoch = baseEpoch + (Number.isFinite(elapsedSince) ? elapsedSince : 0);
          const estimatedUptime = estimateCurrentUptimeMs();
          if (Number.isFinite(estimatedUptime)) {
            const delta = numeric - estimatedUptime;
            return estimatedCurrentEpoch + delta;
          }
          return numeric;
        }
        return numeric;
      }

      function estimateReferenceEpochMs(latestUptimeMs = null) {
        const latestNumeric = Number(latestUptimeMs);
        const hasLatest = Number.isFinite(latestNumeric);
        if (Number.isFinite(currentTimeState.clockOffsetMs)) {
          if (hasLatest) {
            return latestNumeric + currentTimeState.clockOffsetMs;
          }
          const estimatedUptime = estimateCurrentUptimeMs();
          if (Number.isFinite(estimatedUptime)) {
            return estimatedUptime + currentTimeState.clockOffsetMs;
          }
        }
        if (Number.isFinite(currentTimeState.epochSeconds)) {
          const baseEpoch = currentTimeState.epochSeconds * 1000;
          const receivedAt = Number.isFinite(currentTimeState.receivedAtMs)
            ? currentTimeState.receivedAtMs
            : null;
          const elapsedSince = receivedAt !== null ? Date.now() - receivedAt : 0;
          return baseEpoch + (Number.isFinite(elapsedSince) ? elapsedSince : 0);
        }
        if (hasLatest) {
          return latestNumeric;
        }
        const estimatedUptimeFallback = estimateCurrentUptimeMs();
        if (Number.isFinite(estimatedUptimeFallback)) {
          return estimatedUptimeFallback;
        }
        return Date.now();
      }

      function epochToUptimeMs(epochMs) {
        const numeric = Number(epochMs);
        if (!Number.isFinite(numeric)) {
          return null;
        }
        if (Number.isFinite(currentTimeState.clockOffsetMs)) {
          return numeric - currentTimeState.clockOffsetMs;
        }
        const estimatedEpoch = estimateReferenceEpochMs();
        const estimatedUptime = estimateCurrentUptimeMs();
        if (Number.isFinite(estimatedEpoch) && Number.isFinite(estimatedUptime)) {
          const delta = numeric - estimatedEpoch;
          return estimatedUptime + delta;
        }
        return numeric;
      }

      function updatePowerRangeAvailability(meta) {
        const availableCount = Number(meta?.availableCount) || 0;
        let fallbackRange = null;
        powerRangeButtons.forEach((button) => {
          const range = button.dataset.powerRange;
          const enabled = availableCount >= 1 || range === 'current';
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

        const chartSummary = renderPowerChart(
          document.getElementById('powerChart'),
          entries,
          data,
        );

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

        if (filteredCount === 0 || entries.length === 0 || (chartSummary && chartSummary.hasData === false)) {
          powerRangeMessage.textContent =
            selectedPowerRange === 'current'
              ? 'No recent power samples.'
              : 'No power history available yet.';
          return;
        }

        if (selectedPowerRange === 'day') {
          let earliestUptime = null;
          entries.forEach((entry) => {
            const value = Number(entry?.t);
            if (!Number.isFinite(value)) {
              return;
            }
            if (earliestUptime === null || value < earliestUptime) {
              earliestUptime = value;
            }
          });
          const sampleLabel = filteredCount === 1 ? 'sample' : 'samples';
          const hasRealtimeClock =
            Number.isFinite(currentTimeState.clockOffsetMs) ||
            (Number.isFinite(currentTimeState.epochSeconds) &&
              Number.isFinite(currentTimeState.uptimeMs));
          if (earliestUptime !== null) {
            const epochValue = uptimeToEpochMs(earliestUptime);
            if (hasRealtimeClock && Number.isFinite(epochValue)) {
              const formattedDate = new Date(epochValue).toLocaleDateString(undefined, {
                month: 'short',
                day: '2-digit',
                year: 'numeric',
              });
              powerRangeMessage.textContent = `Showing hourly averages for ${formattedDate} (${filteredCount} ${sampleLabel}).`;
              return;
            }
          }
          if (hasRealtimeClock) {
            const fallbackEpoch = estimateReferenceEpochMs(earliestUptime);
            if (Number.isFinite(fallbackEpoch)) {
              const formattedDate = new Date(fallbackEpoch).toLocaleDateString(undefined, {
                month: 'short',
                day: '2-digit',
                year: 'numeric',
              });
              powerRangeMessage.textContent = `Showing hourly averages for ${formattedDate} (${filteredCount} ${sampleLabel}).`;
              return;
            }
          }
          powerRangeMessage.textContent = `Showing hourly averages from the latest 24 hours (${filteredCount} ${sampleLabel}).`;
          return;
        }

        if (selectedPowerRange === 'current') {
          const windowStart = Number(chartSummary.windowStartEpoch);
          const windowEnd = Number(chartSummary.windowEndEpoch);
          const durationMs =
            Number.isFinite(windowStart) && Number.isFinite(windowEnd) && windowEnd >= windowStart
              ? windowEnd - windowStart
              : currentPowerWindowMs;
          const durationText = describeDuration(durationMs);
          powerRangeMessage.textContent = `Showing ${chartSummary.sampleCount} sample${
            chartSummary.sampleCount === 1 ? '' : 's'
          } from the last ${durationText}.`;
          return;
        }

        if (selectedPowerRange === 'week' || selectedPowerRange === 'month') {
          const totalDays = selectedPowerRange === 'week' ? 7 : 30;
          const covered = chartSummary?.coveredBuckets ?? 0;
          const windowStart = chartSummary?.windowStartEpoch;
          const windowEnd = chartSummary?.windowEndEpoch;
          const totalEnergyText = Number.isFinite(chartSummary?.totalWh)
            ? formatEnergyWh(chartSummary.totalWh)
            : null;
          let detail = '';
          if (Number.isFinite(windowStart) && Number.isFinite(windowEnd)) {
            const startLabel = new Date(windowStart).toLocaleDateString(undefined, {
              month: 'short',
              day: '2-digit',
            });
            const endLabel = new Date(windowEnd - 1).toLocaleDateString(undefined, {
              month: 'short',
              day: '2-digit',
            });
            detail = `${startLabel} – ${endLabel}`;
          }
          let message = `Showing daily energy totals for the last ${totalDays} days`;
          if (detail) {
            message += ` (${detail})`;
          }
          message += '.';
          message += ` ${covered} of ${totalDays} day${totalDays === 1 ? '' : 's'} include samples.`;
          if (totalEnergyText) {
            message += ` Total: ${totalEnergyText}.`;
          }
          powerRangeMessage.textContent = message;
          return;
        }

        if (selectedPowerRange === 'year') {
          const covered = chartSummary?.coveredBuckets ?? 0;
          const windowStart = chartSummary?.windowStartEpoch;
          const windowEnd = chartSummary?.windowEndEpoch;
          const totalEnergyText = Number.isFinite(chartSummary?.totalWh)
            ? formatEnergyWh(chartSummary.totalWh)
            : null;
          let detail = '';
          if (Number.isFinite(windowStart) && Number.isFinite(windowEnd)) {
            const startLabel = new Date(windowStart).toLocaleDateString(undefined, {
              month: 'short',
              year: 'numeric',
            });
            const endLabel = new Date(windowEnd - 1).toLocaleDateString(undefined, {
              month: 'short',
              year: 'numeric',
            });
            detail = `${startLabel} – ${endLabel}`;
          }
          let message = 'Showing monthly energy totals for the last 12 months';
          if (detail) {
            message += ` (${detail})`;
          }
          message += '.';
          message += ` ${covered} of 12 months include samples.`;
          if (totalEnergyText) {
            message += ` Total: ${totalEnergyText}.`;
          }
          powerRangeMessage.textContent = message;
          return;
        }

        const coverageMs = Number.isFinite(filteredSpanMs) ? filteredSpanMs : 0;

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
          const duration = powerRangeDurations[selectedPowerRange] || powerRangeDurations.week;
          const params = new URLSearchParams();
          const estimatedUptime = estimateCurrentUptimeMs();
          const canMapEpochToUptime =
            Number.isFinite(currentTimeState.clockOffsetMs) ||
            (Number.isFinite(currentTimeState.epochSeconds) &&
              Number.isFinite(currentTimeState.uptimeMs));

          if (selectedPowerRange === 'day') {
            let dayStartUptime = null;
            let dayEndUptime = null;
            if (canMapEpochToUptime) {
              const referenceEpoch = estimateReferenceEpochMs(estimatedUptime);
              if (Number.isFinite(referenceEpoch)) {
                const dayStart = new Date(referenceEpoch);
                dayStart.setHours(0, 0, 0, 0);
                const dayStartEpoch = dayStart.getTime();
                const dayEndEpoch = dayStartEpoch + 24 * 60 * 60 * 1000 - 1;
                dayStartUptime = epochToUptimeMs(dayStartEpoch);
                dayEndUptime = epochToUptimeMs(dayEndEpoch);
              }
            }
            if (!Number.isFinite(dayStartUptime) && Number.isFinite(estimatedUptime)) {
              dayStartUptime = Math.max(0, estimatedUptime - 24 * 60 * 60 * 1000);
            }
            if (Number.isFinite(dayStartUptime)) {
              params.set('start', String(Math.max(0, Math.floor(dayStartUptime))));
            }
            if (
              Number.isFinite(dayEndUptime) &&
              Number.isFinite(dayStartUptime) &&
              dayEndUptime >= dayStartUptime
            ) {
              params.set('end', String(Math.floor(dayEndUptime)));
            }
          } else if (Number.isFinite(estimatedUptime) && Number.isFinite(duration)) {
            const startCandidate = Math.max(0, estimatedUptime - duration);
            params.set('start', String(Math.floor(startCandidate)));
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
        const uptimeSecondsValue = Number(data.uptimeSeconds);
        const uptimeMs =
          Number.isFinite(uptimeSecondsValue) && uptimeSecondsValue >= 0
            ? uptimeSecondsValue * 1000
            : null;
        const now = Date.now();
        const epochSeconds = Number.isFinite(epochValue) && epochValue > 0 ? epochValue : null;
        const clockOffsetCandidate =
          epochSeconds !== null && Number.isFinite(uptimeMs)
            ? epochSeconds * 1000 - uptimeMs
            : null;
        currentTimeState = {
          epochSeconds,
          receivedAtMs: epochSeconds !== null ? now : null,
          formatted: timeString !== '' ? timeString : null,
          uptimeMs: Number.isFinite(uptimeMs) ? uptimeMs : null,
          uptimeReceivedAtMs: Number.isFinite(uptimeMs) ? now : null,
          clockOffsetMs: Number.isFinite(clockOffsetCandidate) ? clockOffsetCandidate : null,
        };
        if (currentTimeState.formatted === null && currentTimeState.epochSeconds === null) {
          currentTimeState.receivedAtMs = null;
        }
        if (!Number.isFinite(currentTimeState.uptimeMs)) {
          currentTimeState.uptimeMs = null;
          currentTimeState.uptimeReceivedAtMs = null;
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

