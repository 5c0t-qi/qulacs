#pragma once

#include "state.hpp"

#ifdef _USE_GPU

#include <gpusim/util.h>
#include <gpusim/update_ops_cuda.h>

class QuantumStateGpu : public QuantumStateBase {
private:
	GTYPE* _state_vector;
	Random random;
public:
	/**
	 * \~japanese-en �R���X�g���N�^
	 *
	 * @param qubit_count_ �ʎq�r�b�g��
	 */
	QuantumStateGpu(UINT qubit_count_) : QuantumStateBase(qubit_count_) {
		this->_state_vector = reinterpret_cast<CPPCTYPE*>(allocate_quantum_state_host(this->_dim));
		initialize_quantum_state_host(this->data(), _dim);
	}
	/**
	 * \~japanese-en �f�X�g���N�^
	 */
	virtual ~QuantumStateGpu() {
		release_quantum_state(this->data());
	}
	/**
	 * \~japanese-en �ʎq��Ԃ��v�Z����0��Ԃɏ���������
	 */
	virtual void set_zero_state() override {
		initialize_quantum_state_host(this->data(), _dim);
	}
	/**
	 * \~japanese-en �ʎq��Ԃ�<code>comp_basis</code>�̊���Ԃɏ���������
	 *
	 * @param comp_basis �������������\������
	 */
	virtual void set_computational_basis(ITYPE comp_basis)  override {
		set_zero_state();
		_state_vector[0] = make_cuDoubleComplex(0.0, 0.0);
		_state_vector[comp_basis] = make_cuDoubleComplex(1.0, 0.0);
	}
	/**
	 * \~japanese-en �ʎq��Ԃ�Haar random�ɃT���v�����O���ꂽ�ʎq��Ԃɏ���������
	 */
	virtual void set_Haar_random_state() override {
		assert(0);
		// TODO	
	}
	/**
	 * \~japanese-en �ʎq��Ԃ��V�[�h��p����Haar random�ɃT���v�����O���ꂽ�ʎq��Ԃɏ���������
	 */
	virtual void set_Haar_random_state(UINT seed) override {
		assert(0);
		// TODO	
	}
	/**
	 * \~japanese-en <code>target_qubit_index</code>�̓Y�����̗ʎq�r�b�g�𑪒肵�����A0���ϑ������m�����v�Z����B
	 *
	 * �ʎq��Ԃ͕ύX���Ȃ��B
	 * @param target_qubit_index
	 * @return double
	 */
	virtual double get_zero_probability(UINT target_qubit_index) const override {
		return M0_prob_host(target_qubit_index, this->data(), _dim);
	}
	/**
	 * \~japanese-en �����̗ʎq�r�b�g�𑪒肵�����̎��ӊm�����v�Z����
	 *
	 * @param measured_values �ʎq�r�b�g���Ɠ���������0,1,2�̔z��B0,1�͂��̒l���ϑ�����A2�͑�������Ȃ����Ƃ�\���B
	 * @return �v�Z���ꂽ���ӊm��
	 */
	virtual double get_marginal_probability(std::vector<UINT> measured_values) const override {
		std::vector<UINT> target_index;
		std::vector<UINT> target_value;
		for (UINT i = 0; i < measured_values.size(); ++i) {
			if (i == 0 || i == 1) {
				target_index.push_back(i);
				target_value.push_back(measured_values[i]);
			}
		}
		return marginal_prob_host(target_index.data(), target_value.data(), (UINT)target_index.size(), this->data(), _dim);
	}
	/**
	 * \~japanese-en �v�Z���ő��肵����������m�����z�̃G���g���s�[���v�Z����B
	 *
	 * @return �G���g���s�[
	 */
	virtual double get_entropy() const override {
        assert(0);
		// return measurement_distribution_entropy(this->data(), _dim);
	}

	/**
	 * \~japanese-en �ʎq��Ԃ̃m�������v�Z����
	 *
	 * �ʎq��Ԃ̃m�����͔񃆃j�^���ȃQ�[�g����p�������ɏ������Ȃ�B
	 * @return �m����
	 */
	virtual double get_norm() const override {
		return state_norm_host(this->data(), _dim);
	}

	/**
	 * \~japanese-en �ʎq��Ԃ𐳋K������
	 *
	 * @param norm ���g�̃m����
	 */
	virtual void normalize(double norm) override {
		normalize_host(norm, this->data(), _dim);
	}


	/**
	 * \~japanese-en �o�b�t�@�Ƃ��ē����T�C�Y�̗ʎq��Ԃ��쐬����B
	 *
	 * @return �������ꂽ�ʎq���
	 */
	virtual QuantumStateBase* allocate_buffer() const override {
		QuantumStateGpu* new_state = new QuantumStateGpu(this->_qubit_count);
		return new_state;
	}
	/**
	 * \~japanese-en ���g�̏�Ԃ̃f�B�[�v�R�s�[�𐶐�����
	 *
	 * @return ���g�̃f�B�[�v�R�s�[
	 */
	virtual QuantumStateBase* copy() const override {
		QUantumnStateGpu* new_state = new QUantumnStateGpu(this->_qubit_count);
		memcpy(new_state->data_cpp(), _state_vector, (size_t)(sizeof(CPPCTYPE)*_dim));
		for (UINT i = 0; i < _classical_register.size(); ++i) new_state->set_classical_value(i, _classical_register[i]);
		return new_state;
	}
	/**
	 * \~japanese-en <code>state</code>�̗ʎq��Ԃ����g�փR�s�[����B
	 */
	virtual void load(QuantumStateBase* _state) {
		this->_classical_register = _state->classical_register;
		memcpy(this->data_cpp(), _state->data_cpp(), (size_t)(sizeof(CPPCTYPE)*_dim));
	}
	/**
	 * \~japanese-en �ʎq��Ԃ��z�u����Ă��郁������ێ�����f�o�C�X�����擾����B
	 */
	virtual const char* get_device_name() const override { return "gpu"; }
	/**
	 * \~japanese-en �ʎq��Ԃ�C++��<code>std::complex\<double\></code>�̔z��Ƃ��Ď擾����
	 *
	 * @return ���f�x�N�g���̃|�C���^
	 */
	virtual CPPCTYPE* data_cpp() const override { return this->_state_vector; }
	/**
	 * \~japanese-en �ʎq��Ԃ�csim��Complex�^�̔z��Ƃ��Ď擾����
	 *
	 * @return ���f�x�N�g���̃|�C���^
	 */
	virtual void* data() const override {
		return reinterpret_cast<void*>(this->_state_vector);
	}

	/**
	 * \~japanese-en �ʎq��Ԃ𑪒肵���ۂ̌v�Z���̃T���v�����O���s��
	 *
	 * @param[in] sampling_count �T���v�����O���s����
	 * @return �T���v�����ꂽ�l�̃��X�g
	 */
	virtual std::vector<ITYPE> sampling(UINT sampling_count) override {
		std::vector<double> stacked_prob;
		std::vector<ITYPE> result;
		double sum = 0.;
		auto ptr = this->data_cpp();
		stacked_prob.push_back(0.);
		for (UINT i = 0; i < this->dim; ++i) {
			sum += norm(ptr[i]);
			stacked_prob.push_back(sum);
		}

		for (UINT count = 0; count < sampling_count; ++count) {
			double r = random.uniform();
			auto ite = std::lower_bound(stacked_prob.begin(), stacked_prob.end(), r);
			auto index = std::distance(stacked_prob.begin(), ite) - 1;
			result.push_back(index);
		}
		return result;
	}
};


#endif // _USE_GPU
