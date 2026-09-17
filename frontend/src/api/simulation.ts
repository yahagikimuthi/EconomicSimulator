export interface SimConfig {
  agentCount: number;
  taxRate: number;
}

export interface SimResponse {
  status: string;
  gdp: number;
  history?: number[];
}

export const runSimulation = async (config: SimConfig): Promise<SimResponse> => {
  const response = await fetch('http://localhost:18080/api/simulate', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({
      agent_count: config.agentCount,
      tax_rate: config.taxRate,
    }),
  });

  if (!response.ok) {
    throw new Error('Simulation API request failed');
  }

  return response.json();
};