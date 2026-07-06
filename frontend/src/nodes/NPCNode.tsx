import { memo, useState } from 'react';
import { Handle, Position } from 'reactflow';
import { User, Brain, Users, Eye } from 'lucide-react';
import { NPC } from '../types';
import './NPCNode.css';

interface NPCNodeProps {
  data: { npc: NPC };
  isconnecting: boolean;
  selected: boolean;
}

export const NPCNode = memo(({ data, isconnecting, selected }: NPCNodeProps) => {
  const { npc } = data;
  const [expanded, setExpanded] = useState(false);

  return (
    <div className={`npc-node ${selected ? 'selected' : ''} ${expanded ? 'expanded' : ''}`}>
      <Handle type="target" position={Position.Top} />

      <div className="npc-header" onClick={() => setExpanded(!expanded)}>
        <div className="npc-title">
          <User size={16} />
          <span className="npc-name">{npc.name}</span>
        </div>
        <div className="npc-age">{npc.age}</div>
      </div>

      {expanded && (
        <div className="npc-details">
          <div className="detail-item">
            <span className="label">Race:</span>
            <span>{npc.race} {npc.subrace && `(${npc.subrace})`}</span>
          </div>
          <div className="detail-item">
            <span className="label">Occupation:</span>
            <span>{npc.occupation}</span>
          </div>
          <div className="detail-item">
            <span className="label">Gender:</span>
            <span>{npc.gender}</span>
          </div>
          <div className="detail-item">
            <div className="label">
              <Brain size={14} /> Sanity
            </div>
            <span className="sanity">{npc.sanity_points}/100</span>
          </div>
          <div className="detail-item">
            <span className="label">Clothing:</span>
            <span>{npc.clothing_style}</span>
          </div>
          <div className="detail-item">
            <div className="label">
              <Users size={14} /> Personality
            </div>
            <span className="personality">{npc.personality}</span>
          </div>
          <div className="detail-item secret">
            <div className="label">
              <Eye size={14} /> Secret
            </div>
            <span>{npc.secret}</span>
          </div>
        </div>
      )}

      <Handle
        type="source"
        position={Position.Bottom}
        isconnecting={isconnecting}
      />
    </div>
  );
});

NPCNode.displayName = 'NPCNode';
