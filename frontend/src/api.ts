import { NPC } from './types';

const API_BASE = import.meta.env.VITE_API_BASE || 'http://localhost:8080';

export const generateNPC = async (): Promise<NPC> => {
  const response = await fetch(`${API_BASE}/npc/random`);
  if (!response.ok) throw new Error('Failed to fetch NPC');
  return response.json();
};

export const generateNPCWithSeed = async (seed: number): Promise<NPC> => {
  const response = await fetch(`${API_BASE}/npc`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ seed }),
  });
  if (!response.ok) throw new Error('Failed to fetch NPC');
  return response.json();
};

export const generateNPCsForWorld = async (count: number = 5): Promise<NPC[]> => {
  const npcs: NPC[] = [];
  for (let i = 0; i < count; i++) {
    try {
      const npc = await generateNPC();
      npcs.push(npc);
    } catch (err) {
      console.error(`Failed to generate NPC ${i}:`, err);
    }
  }
  return npcs;
};

export const fetchWorld = async (): Promise<{ npcs: (NPC & { id: string })[]; relationships: unknown[] }> => {
  const response = await fetch(`${API_BASE}/world`);
  if (!response.ok) throw new Error('Failed to fetch world');
  return response.json();
};

export const loadPlayerBoard = async () => {
  try {
    const response = await fetch(`${API_BASE}/board`);
    if (response.ok) {
      return response.json();
    }
  } catch (err) {
    console.error('Failed to load player board:', err);
  }
  return null;
};

export const savePlayerBoard = async (boardData: unknown) => {
  try {
    const response = await fetch(`${API_BASE}/board`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(boardData),
    });
    if (!response.ok) throw new Error('Failed to save board');
    return response.json();
  } catch (err) {
    console.error('Failed to save player board:', err);
    return null;
  }
};
