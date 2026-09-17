import React, { useState } from 'react';
import {
  Container,
  Box,
  Typography,
  Slider,
  Button,
  Card,
  CardContent,
  CircularProgress,
  Grid,
} from '@mui/material';
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
} from 'recharts';
import { runSimulation, SimConfig } from './api/simulation';

interface ChartDataPoint {
  step: number;
  gdp: number;
}

export const App: React.FC = () => {
  const [config, setConfig] = useState<SimConfig>({ agentCount: 100, taxRate: 0.1 });
  const [loading, setLoading] = useState<boolean>(false);
  const [chartData, setChartData] = useState<ChartDataPoint[]>([]);

  const handleRun = async () => {
    setLoading(true);
    try {
      const result = await runSimulation(config);
      
      // レスポンスの history 配列、または単一 GDP 値からグラフ用データを生成
      if (result.history) {
        const formatted = result.history.map((val, idx) => ({ step: idx, gdp: val }));
        setChartData(formatted);
      } else {
        setChartData([{ step: 1, gdp: result.gdp }]);
      }
    } catch (error) {
      console.error('API Error:', error);
    } finally {
      setLoading(false);
    }
  };

  return (
    <Container maxWidth="lg" sx={{ py: 4 }}>
      <Typography variant="h4" gutterBottom fontWeight="bold">
        経済シミュレーション ダッシュボード
      </Typography>

      <Grid container spacing={3}>
        {/* コントロールパネル */}
        <Grid item xs={12} md={4}>
          <Card variant="outlined">
            <CardContent>
              <Typography variant="h6" gutterBottom>
                パラメータ設定
              </Typography>

              <Box sx={{ my: 3 }}>
                <Typography id="agent-slider" gutterBottom>
                  エージェント数: {config.agentCount}
                </Typography>
                <Slider
                  value={config.agentCount}
                  min={10}
                  max={1000}
                  step={10}
                  onChange={(_, val) => setConfig({ ...config, agentCount: val as number })}
                />
              </Box>

              <Box sx={{ my: 3 }}>
                <Typography id="tax-slider" gutterBottom>
                  税率: {(config.taxRate * 100).toFixed(0)}%
                </Typography>
                <Slider
                  value={config.taxRate}
                  min={0}
                  max={0.5}
                  step={0.01}
                  onChange={(_, val) => setConfig({ ...config, taxRate: val as number })}
                />
              </Box>

              <Button
                variant="contained"
                fullWidth
                size="large"
                onClick={handleRun}
                disabled={loading}
              >
                {loading ? <CircularProgress size={24} /> : 'シミュレーション実行'}
              </Button>
            </CardContent>
          </Card>
        </Grid>

        {/* グラフ描画エリア */}
        <Grid item xs={12} md={8}>
          <Card variant="outlined" sx={{ p: 2 }}>
            <Typography variant="h6" gutterBottom>
              GDP 推移
            </Typography>
            <Box sx={{ height: 400, width: '100%' }}>
              {chartData.length > 0 ? (
                <ResponsiveContainer width="100%" height="100%">
                  <LineChart data={chartData}>
                    <CartesianGrid strokeDasharray="3 3" />
                    <XAxis dataKey="step" />
                    <YAxis />
                    <Tooltip />
                    <Line type="monotone" dataKey="gdp" stroke="#1976d2" strokeWidth={2} />
                  </LineChart>
                </ResponsiveContainer>
              ) : (
                <Box
                  display="flex"
                  alignItems="center"
                  justifyContent="center"
                  height="100%"
                >
                  <Typography color="text.secondary">
                    パラメータを指定して実行ボタンを押してください
                  </Typography>
                </Box>
              )}
            </Box>
          </Card>
        </Grid>
      </Grid>
    </Container>
  );
};

export default App;