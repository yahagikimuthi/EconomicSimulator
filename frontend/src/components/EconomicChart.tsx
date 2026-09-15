import React from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts';

// ダミーデータ（本来は WASM / シミュレータから受け取るデータ）
const data = [
  { tick: 1, gdp: 1000, inflation: 1.0 },
  { tick: 2, gdp: 1050, inflation: 1.2 },
  { tick: 3, gdp: 1100, inflation: 1.1 },
  { tick: 4, gdp: 1080, inflation: 1.5 },
  { tick: 5, gdp: 1150, inflation: 1.4 },
];

export const EconomicChart: React.FC = () => {
  return (
    <div style={{ width: '100%', height: 400, padding: '20px' }}>
      <h2>マクロ経済指標推移 (Dummy)</h2>
      <ResponsiveContainer width="100%" height="100%">
        <LineChart data={data}>
          <CartesianGrid strokeDasharray="3 3" />
          <XAxis dataKey="tick" label={{ value: 'Tick', position: 'insideBottom', offset: -5 }} />
          <YAxis yAxisId="left" label={{ value: 'GDP', angle: -90, position: 'insideLeft' }} />
          <YAxis yAxisId="right" orientation="right" label={{ value: 'インフレ率 (%)', angle: 90, position: 'insideRight' }} />
          <Tooltip />
          <Legend />
          <Line yAxisId="left" type="monotone" dataKey="gdp" stroke="#8884d8" name="GDP" activeDot={{ r: 8 }} />
          <Line yAxisId="right" type="monotone" dataKey="inflation" stroke="#82ca9d" name="インフレ率" />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
};