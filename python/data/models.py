from typing import List, Optional, Dict, Any, Union
from pydantic import BaseModel, Field, field_validator
from uuid import UUID, uuid4
from enum import Enum
from datetime import datetime


class Point(BaseModel):
    x: float
    y: float
    z: Optional[float] = 0.0


class Color(BaseModel):
    r: int = Field(ge=0, le=255)
    g: int = Field(ge=0, le=255)
    b: int = Field(ge=0, le=255)
    a: float = Field(ge=0.0, le=1.0, default=1.0)
    
    @field_validator('r', 'g', 'b')
    def validate_rgb(cls, v):
        if not 0 <= v <= 255:
            raise ValueError('RGB values must be between 0 and 255')
        return v
    
    def to_hex(self) -> str:
        return f"#{self.r:02x}{self.g:02x}{self.b:02x}"
    
    @classmethod
    def from_hex(cls, hex_color: str) -> 'Color':
        hex_color = hex_color.lstrip('#')
        return cls(
            r=int(hex_color[0:2], 16),
            g=int(hex_color[2:4], 16),
            b=int(hex_color[4:6], 16)
        )


class BoundingBox(BaseModel):
    min_x: float
    min_y: float
    max_x: float
    max_y: float
    
    @property
    def width(self) -> float:
        return self.max_x - self.min_x
    
    @property
    def height(self) -> float:
        return self.max_y - self.min_y
    
    @property
    def center(self) -> Point:
        return Point(
            x=(self.min_x + self.max_x) / 2,
            y=(self.min_y + self.max_y) / 2
        )


class Transform(BaseModel):
    translate_x: float = 0.0
    translate_y: float = 0.0
    translate_z: float = 0.0
    rotate_x: float = 0.0
    rotate_y: float = 0.0
    rotate_z: float = 0.0
    scale_x: float = 1.0
    scale_y: float = 1.0
    scale_z: float = 1.0


class LineStyle(str, Enum):
    SOLID = "solid"
    DASHED = "dashed"
    DOTTED = "dotted"
    DASH_DOT = "dash_dot"


class TextAlignment(str, Enum):
    LEFT = "left"
    CENTER = "center"
    RIGHT = "right"


class VerticalAlignment(str, Enum):
    TOP = "top"
    MIDDLE = "middle"
    BOTTOM = "bottom"


class GradientType(str, Enum):
    LINEAR = "linear"
    RADIAL = "radial"


class GradientStop(BaseModel):
    offset: float = Field(ge=0.0, le=1.0)
    color: Color


class Gradient(BaseModel):
    type: GradientType
    stops: List[GradientStop]
    angle: Optional[float] = 0.0  # For linear gradients
    center: Optional[Point] = None  # For radial gradients
    radius: Optional[float] = None  # For radial gradients


class FillStyle(BaseModel):
    color: Optional[Color] = None
    gradient: Optional[Gradient] = None
    pattern: Optional[str] = None  # Pattern ID reference


class DrawableObject(BaseModel):
    id: UUID = Field(default_factory=uuid4)
    name: Optional[str] = None
    stroke_color: Optional[Color] = Color(r=0, g=0, b=0)
    stroke_width: float = Field(ge=0.0, default=1.0)
    fill: Optional[FillStyle] = None
    opacity: float = Field(ge=0.0, le=1.0, default=1.0)
    visible: bool = True
    locked: bool = False
    layer_id: Optional[UUID] = None
    transform: Transform = Field(default_factory=Transform)
    metadata: Dict[str, Any] = Field(default_factory=dict)
    created_at: datetime = Field(default_factory=datetime.now)
    updated_at: datetime = Field(default_factory=datetime.now)
    
    def get_bounding_box(self) -> BoundingBox:
        raise NotImplementedError("Subclasses must implement get_bounding_box")


class Line(DrawableObject):
    start_point: Point
    end_point: Point
    line_style: LineStyle = LineStyle.SOLID
    
    def get_bounding_box(self) -> BoundingBox:
        return BoundingBox(
            min_x=min(self.start_point.x, self.end_point.x),
            min_y=min(self.start_point.y, self.end_point.y),
            max_x=max(self.start_point.x, self.end_point.x),
            max_y=max(self.start_point.y, self.end_point.y)
        )


class Circle(DrawableObject):
    center: Point
    radius: float = Field(gt=0.0)
    
    def get_bounding_box(self) -> BoundingBox:
        return BoundingBox(
            min_x=self.center.x - self.radius,
            min_y=self.center.y - self.radius,
            max_x=self.center.x + self.radius,
            max_y=self.center.y + self.radius
        )


class Ellipse(DrawableObject):
    center: Point
    rx: float = Field(gt=0.0)
    ry: float = Field(gt=0.0)
    rotation: float = 0.0
    
    def get_bounding_box(self) -> BoundingBox:
        # Simplified bounding box (doesn't account for rotation)
        return BoundingBox(
            min_x=self.center.x - self.rx,
            min_y=self.center.y - self.ry,
            max_x=self.center.x + self.rx,
            max_y=self.center.y + self.ry
        )


class Rectangle(DrawableObject):
    top_left: Point
    width: float = Field(gt=0.0)
    height: float = Field(gt=0.0)
    corner_radius: float = Field(ge=0.0, default=0.0)
    
    def get_bounding_box(self) -> BoundingBox:
        return BoundingBox(
            min_x=self.top_left.x,
            min_y=self.top_left.y,
            max_x=self.top_left.x + self.width,
            max_y=self.top_left.y + self.height
        )


class Polygon(DrawableObject):
    points: List[Point] = Field(min_length=3)
    closed: bool = True
    
    def get_bounding_box(self) -> BoundingBox:
        x_coords = [p.x for p in self.points]
        y_coords = [p.y for p in self.points]
        return BoundingBox(
            min_x=min(x_coords),
            min_y=min(y_coords),
            max_x=max(x_coords),
            max_y=max(y_coords)
        )


class Polyline(DrawableObject):
    points: List[Point] = Field(min_length=2)
    line_style: LineStyle = LineStyle.SOLID
    
    def get_bounding_box(self) -> BoundingBox:
        x_coords = [p.x for p in self.points]
        y_coords = [p.y for p in self.points]
        return BoundingBox(
            min_x=min(x_coords),
            min_y=min(y_coords),
            max_x=max(x_coords),
            max_y=max(y_coords)
        )


class Arc(DrawableObject):
    center: Point
    radius: float = Field(gt=0.0)
    start_angle: float  # in degrees
    end_angle: float    # in degrees
    
    def get_bounding_box(self) -> BoundingBox:
        # Simplified bounding box
        return BoundingBox(
            min_x=self.center.x - self.radius,
            min_y=self.center.y - self.radius,
            max_x=self.center.x + self.radius,
            max_y=self.center.y + self.radius
        )


class Text(DrawableObject):
    position: Point
    content: str
    font_family: str = "Arial"
    font_size: float = Field(gt=0.0, default=12.0)
    font_weight: str = "normal"
    font_style: str = "normal"
    text_alignment: TextAlignment = TextAlignment.LEFT
    vertical_alignment: VerticalAlignment = VerticalAlignment.BOTTOM
    
    def get_bounding_box(self) -> BoundingBox:
        # Approximate bounding box based on font size
        # In a real implementation, this would use font metrics
        estimated_width = len(self.content) * self.font_size * 0.6
        estimated_height = self.font_size * 1.2
        
        return BoundingBox(
            min_x=self.position.x,
            min_y=self.position.y - estimated_height,
            max_x=self.position.x + estimated_width,
            max_y=self.position.y
        )


class PathCommand(BaseModel):
    command: str  # M, L, C, Q, A, Z, etc.
    params: List[float] = Field(default_factory=list)


class Path(DrawableObject):
    commands: List[PathCommand]
    
    def get_bounding_box(self) -> BoundingBox:
        # Simplified implementation
        # In reality, this would parse the path commands
        return BoundingBox(min_x=0, min_y=0, max_x=100, max_y=100)


# Union type for all drawable objects
DrawableObjectType = Union[
    Line, Circle, Ellipse, Rectangle, Polygon, 
    Polyline, Arc, Text, Path
]


class Group(DrawableObject):
    objects: List[DrawableObjectType] = Field(default_factory=list)
    
    def get_bounding_box(self) -> BoundingBox:
        if not self.objects:
            return BoundingBox(min_x=0, min_y=0, max_x=0, max_y=0)
        
        boxes = [obj.get_bounding_box() for obj in self.objects]
        return BoundingBox(
            min_x=min(box.min_x for box in boxes),
            min_y=min(box.min_y for box in boxes),
            max_x=max(box.max_x for box in boxes),
            max_y=max(box.max_y for box in boxes)
        )
    
    def add_object(self, obj: DrawableObjectType):
        self.objects.append(obj)
        self.updated_at = datetime.now()
    
    def remove_object(self, obj_id: UUID) -> bool:
        initial_length = len(self.objects)
        self.objects = [obj for obj in self.objects if obj.id != obj_id]
        if len(self.objects) < initial_length:
            self.updated_at = datetime.now()
            return True
        return False


class Layer(BaseModel):
    id: UUID = Field(default_factory=uuid4)
    name: str
    visible: bool = True
    locked: bool = False
    opacity: float = Field(ge=0.0, le=1.0, default=1.0)
    objects: List[Union[DrawableObjectType, Group]] = Field(default_factory=list)
    z_index: int = 0
    
    def add_object(self, obj: Union[DrawableObjectType, Group]):
        obj.layer_id = self.id
        self.objects.append(obj)
    
    def remove_object(self, obj_id: UUID) -> bool:
        initial_length = len(self.objects)
        self.objects = [obj for obj in self.objects if obj.id != obj_id]
        return len(self.objects) < initial_length
    
    def get_object_by_id(self, obj_id: UUID) -> Optional[Union[DrawableObjectType, Group]]:
        for obj in self.objects:
            if obj.id == obj_id:
                return obj
            if isinstance(obj, Group):
                for sub_obj in obj.objects:
                    if sub_obj.id == obj_id:
                        return sub_obj
        return None


class Drawing(BaseModel):
    id: UUID = Field(default_factory=uuid4)
    name: str = "Untitled Drawing"
    width: float = Field(gt=0.0, default=800)
    height: float = Field(gt=0.0, default=600)
    background_color: Color = Field(default_factory=lambda: Color(r=255, g=255, b=255))
    layers: List[Layer] = Field(default_factory=list)
    metadata: Dict[str, Any] = Field(default_factory=dict)
    created_at: datetime = Field(default_factory=datetime.now)
    updated_at: datetime = Field(default_factory=datetime.now)
    
    def add_layer(self, layer: Layer):
        self.layers.append(layer)
        self.updated_at = datetime.now()
    
    def remove_layer(self, layer_id: UUID) -> bool:
        initial_length = len(self.layers)
        self.layers = [layer for layer in self.layers if layer.id != layer_id]
        if len(self.layers) < initial_length:
            self.updated_at = datetime.now()
            return True
        return False
    
    def get_layer_by_id(self, layer_id: UUID) -> Optional[Layer]:
        for layer in self.layers:
            if layer.id == layer_id:
                return layer
        return None
    
    def get_all_objects(self) -> List[Union[DrawableObjectType, Group]]:
        all_objects = []
        for layer in self.layers:
            all_objects.extend(layer.objects)
        return all_objects
    
    def to_svg(self) -> str:
        """Convert drawing to SVG format"""
        from python.data.svg_renderer import drawing_to_svg
        return drawing_to_svg(self)