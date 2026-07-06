import { memo } from 'react';
import { Handle, Position } from 'reactflow';
import { Building2 } from 'lucide-react';
import './BuildingNode.css';

interface BuildingNodeProps {
  data: { name: string; type?: string };
  selected: boolean;
}

export const BuildingNode = memo(({ data, selected }: BuildingNodeProps) => {
  return (
    <div className={`building-node ${selected ? 'selected' : ''}`}>
      <Handle type="target" position={Position.Top} />
      <div className="building-content">
        <Building2 size={20} />
        <div className="building-label">{data.name}</div>
        {data.type && <div className="building-type">{data.type}</div>}
      </div>
      <Handle type="source" position={Position.Bottom} />
    </div>
  );
});

BuildingNode.displayName = 'BuildingNode';
