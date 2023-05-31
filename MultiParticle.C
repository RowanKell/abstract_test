#include "MultiParticle.h"
MultiParticle::update(int _id, int _pid, double _px, double _py, double _pz, int _daughter, int _parent, double _mass, double _vz)
{
    v_id.push_back(_id);
    v_pid.push_back(_pid);
    v_px.push_back(_px);
    v_py.push_back(_py);
    v_pz.push_back(_pz);
    v_daughter.push_back(_daughter);
    v_parent.push_back(_parent);
    v_mass.push_back(_mass);
    v_vz.push_back(_vz);
}