// Backend API types
export interface NPC {
  name: string;
  occupation: string;
  age: number;
  gender: string;
  race: string;
  subrace: string;
  sanity_points: number;
  clothing_style: string;
  personality: string;
  secret: string;
  log?: string[];
}

export interface Building {
  id: string;
  name: string;
  type: string;
  description?: string;
}

export interface NPCNode {
  id: string;
  type: 'npc';
  npc: NPC;
}

export interface BuildingNode {
  id: string;
  type: 'building';
  building: Building;
}

export interface EvidenceNode {
  id: string;
  type: 'evidence';
  title: string;
  description: string;
}

export interface NoteNode {
  id: string;
  type: 'note';
  title: string;
  content: string;
}

export type WorldNode = NPCNode | BuildingNode | EvidenceNode | NoteNode;

export interface Relationship {
  from: string;
  to: string;
  type: 'family' | 'friend' | 'enemy' | 'employer' | 'lives_at' | 'seen_with' | 'owns' | 'owes_money' | 'murdered' | 'witnessed' | 'investigating' | 'member_of' | 'unknown';
  label: string;
}

export interface WorldState {
  nodes: WorldNode[];
  relationships: Relationship[];
}

export const relationshipColors: Record<string, string> = {
  family: '#ef4444',
  friend: '#3b82f6',
  enemy: '#f59e0b',
  employer: '#10b981',
  lives_at: '#8b5cf6',
  seen_with: '#ec4899',
  owns: '#06b6d4',
  owes_money: '#f97316',
  murdered: '#7c2d12',
  witnessed: '#0f766e',
  investigating: '#4f46e5',
  member_of: '#6d28d9',
  unknown: '#6b7280',
};
