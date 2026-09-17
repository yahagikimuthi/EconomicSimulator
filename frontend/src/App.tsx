import { useState } from 'react';
import { 
  Container, 
  Box, 
  Typography, 
  Button, 
  Slider, 
  Stack, 
  Paper, 
  Grid 
} from '@mui/material';
import PlayArrowIcon from '@mui/icons-material/PlayArrow';
import PauseIcon from '@mui/icons-material/Pause';
import RestartAltIcon from '@mui/icons-material/RestartAlt';
import { EconomicChart } from './components/EconomicChart';

function AgentSlider(agent: number, setAgentCount: any) {
  return (
    <Box>
      <Typography id="agent-slider" variant="body2" color="text.secondary">
        エージェント数: {agent}
      </Typography>
      <Slider
      value={agent}
      onChange={(_, val) => {setAgentCount(val)}}
      min={10}
      max={1000}
      step={10}
      valueLabelDisplay="auto"
      />
    </Box>
  )
}

function TaxSlider(rate: number, setTaxRate: any) {
  return (
    <Box>
      <Typography id="tax-slider" variant="body2" color="text.secondary">
        税率: {rate}
      </Typography>
      <Slider value={rate}
      onChange={(_, val) => setTaxRate(val as number)}
      min={0}
      max={50}
      step={1}
      valueLabelDisplay="auto"
      />
    </Box>
  )
}

export default function App() {
  const [isRunning, setIsRunning] = useState(false);
  const [agentCount, setAgentCount] = useState<number>(100);
  const [taxRate, setTaxRate] = useState<number>(10);

  return (
    <Container maxWidth="lg" sx={{ mt: 4, mb: 4 }}>
      <Typography variant="h4" component="h1" gutterBottom fontWeight="bold">
        Economic Simulator Control Panel
      </Typography>

      <Grid container spacing={3}>
        {/* 左側: コントロールパネル */}
        <Grid item xs={12} md={4}>
          <Paper elevation={3} sx={{ p: 3 }}>
            <Typography variant="h6" gutterBottom>
              シミュレーション設定
            </Typography>
            
            <Stack spacing={3} sx={{ mt: 2 }}>
              <AgentSlider agent={agentCount} setAgentCount={setAgentCount}/>
              <TaxSlider rate={taxRate} setTaxRate={setTaxRate}/>

              {/* 操作ボタン */}
              <Stack direction="row" spacing={1}>
                <Button
                  variant="contained"
                  color={isRunning ? "warning" : "primary"}
                  startIcon={isRunning ? <PauseIcon /> : <PlayArrowIcon />}
                  onClick={() => setIsRunning(!isRunning)}
                  fullWidth
                >
                  {isRunning ? "一時停止" : "開始"}
                </Button>
                <Button 
                  variant="outlined" 
                  color="error" 
                  startIcon={<RestartAltIcon />}
                  onClick={() => setIsRunning(false)}
                >
                  リセット
                </Button>
              </Stack>
            </Stack>
          </Paper>
        </Grid>

        {/* 右側: グラフ表示領域 */}
        <Grid item xs={12} md={8}>
          <Paper elevation={3} sx={{ p: 2 }}>
            <EconomicChart />
          </Paper>
        </Grid>
      </Grid>
    </Container>
  );
}