from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import Column, ForeignKey
from sqlalchemy.types import Boolean, CHAR, VARCHAR, Integer, String, DateTime, Enum, UnicodeText, TIMESTAMP
from sqlalchemy.sql import functions as func

from sqlalchemy.orm import relationship

BaseModel = declarative_base()


class Box(BaseModel):
    __tablename__ = 'Box'

    mac = Column(VARCHAR(17), primary_key=True, nullable=False)
    cid = Column(VARCHAR(8), unique=True, nullable=False)
    area = Column(VARCHAR(10), nullable=False)
    create_time = Column(DateTime, default=func.current_timestamp(), nullable=False)
    active_time = Column(Integer, default=0, nullable=False)
    cert_eigenvalue = Column(String(255),  default=func.current_timestamp(), nullable=False)
    cert_path = Column(VARCHAR(255), nullable=False)
    server_ip = Column(VARCHAR(15), ForeignKey('Server.ip'), nullable=False,  primary_key=True)

    def __repr__(self):
        return '<Box mac : %r, cid : %r, area: %r, create_time: %r>' % (self.mac, self.cid, self.area, self.create_time)

    def get_eval(self, attr, default=None):
        if hasattr(self, attr) and getattr(self, attr):
            return eval(getattr(self, attr))
        else:
            return default

class Server(BaseModel):
    """docstring for Initdata"""
    __tablename__ = 'Server'

    ip = Column(VARCHAR(15), primary_key=True, nullable=False)
    area = Column(VARCHAR(10), nullable=False)

    #ips = relationship('Server')

    def __repr__(self):
        return '<Server ip : %r, area : %r>' % (self.ip, self.area)


def init_db(engine):
    BaseModel.metadata.create_all(engine)


def drop_db(engine):
    BaseModel.metadata.drop_all(engine)
